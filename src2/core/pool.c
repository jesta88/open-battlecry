#include "../../include/engine/pool.h"

#include <assert.h>
#include <mimalloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <intrin.h>
    #include <windows.h>
    #define POOL_CACHE_LINE 64
#else
    #include <unistd.h>
    #define POOL_CACHE_LINE 64
#endif

// Memory tagging for debugging
#define POOL_FREE_PATTERN 0xDEADBEEF
#define POOL_ALLOC_PATTERN 0xABCDEF00

typedef struct pool_free_node
{
    struct pool_free_node* next;
#if POOL_DEBUG
    uint32_t magic; // Detect double-free
#endif
} pool_free_node_t;

// Memory block containing multiple objects
struct pool_block
{
    pool_block_t* next;
    pool_block_t* prev;
    pool_allocator_t* owner;
    u64 num_objects;
    u64 objects_in_use;
    uint8_t* memory;

    // Embedded free list for this block
    pool_free_node_t* free_list;

#if POOL_DEBUG
    uint64_t* allocation_bitmap; // Track which slots are allocated
#endif
};

// Pool allocator structure
struct pool_allocator
{
    // Basic properties
    u64 object_size;       // Size of each object
    u64 object_alignment;  // Alignment requirement
    u64 objects_per_block; // Objects in each block

    // Block management
    pool_block_t* blocks;       // Linked list of all blocks
    pool_block_t* active_block; // Current block for allocations
    u64 block_count;

    // Free list management
    pool_free_node_t* global_free_list; // Cross-block free list

    // Memory management
    mi_heap_t* heap;       // mimalloc heap for isolation
    bool use_thread_local; // Use thread-local allocation

    // Statistics
    u64 total_objects;
    u64 allocated_objects;
    u64 peak_allocated;

#if POOL_DEBUG
    const char* name;
    u64 allocation_count;
    u64 deallocation_count;
    bool detect_double_free;
    bool clear_on_free;
#endif

    // Cache-line padding to avoid false sharing
    WAR_ALIGN(POOL_CACHE_LINE) char padding[POOL_CACHE_LINE];
};

// Fast bit scanning for free slot finding
WAR_FORCE_INLINE static int pool_find_first_set(uint64_t mask)
{
#if defined(_MSC_VER)
    unsigned long index;
    _BitScanForward64(&index, mask);
    return (int) index;
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_ctzll(mask);
#else
    // Fallback implementation
    int pos = 0;
    while ((mask & 1) == 0)
    {
        mask >>= 1;
        pos++;
    }
    return pos;
#endif
}

static pool_block_t* pool_block_create(pool_allocator_t* pool)
{
    const u64 object_size = war_align_up(pool->object_size, pool->object_alignment);
    const u64 block_data_size = object_size * pool->objects_per_block;

    // Allocate block header and data together
    u64 total_size = sizeof(pool_block_t) + block_data_size;

#if POOL_DEBUG
    // Add space for allocation bitmap
    const u64 bitmap_size = (pool->objects_per_block + 63) / 64 * sizeof(uint64_t);
    total_size += bitmap_size;
#endif

    pool_block_t* block = mi_heap_calloc(pool->heap, 1, total_size);
    if (!block)
        return NULL;

    block->owner = pool;
    block->num_objects = pool->objects_per_block;
    block->objects_in_use = 0;
    block->memory = (uint8_t*) (block + 1);

#if POOL_DEBUG
    block->allocation_bitmap = (uint64_t*) (block->memory + block_data_size);
    memset(block->allocation_bitmap, 0, bitmap_size);
#endif

    // Initialize free list for this block
    pool_free_node_t* prev = NULL;
    for (u64 i = 0; i < pool->objects_per_block; i++)
    {
        pool_free_node_t* node = (pool_free_node_t*) (block->memory + i * object_size);
        node->next = prev;
#if POOL_DEBUG
        node->magic = POOL_FREE_PATTERN;
#endif
        prev = node;
    }
    block->free_list = prev;

    // Link into pool's block list
    block->next = pool->blocks;
    block->prev = NULL;
    if (pool->blocks)
    {
        pool->blocks->prev = block;
    }
    pool->blocks = block;
    pool->block_count++;
    pool->total_objects += pool->objects_per_block;

    return block;
}

static void pool_block_destroy(pool_allocator_t* pool, pool_block_t* block)
{
    // Unlink from list
    if (block->prev)
    {
        block->prev->next = block->next;
    }
    else
    {
        pool->blocks = block->next;
    }
    if (block->next)
    {
        block->next->prev = block->prev;
    }

    pool->total_objects -= block->num_objects;
    pool->block_count--;

    mi_free(block);
}

pool_allocator_t* pool_create_ex(const pool_config_t* config)
{
    if (config->object_size == 0)
        return NULL;

    // Calculate aligned object size
    u64 alignment = config->object_alignment;
    if (alignment == 0)
    {
        // Default alignment based on size
        if (config->object_size >= 16)
            alignment = 16;
        else if (config->object_size >= 8)
            alignment = 8;
        else
            alignment = sizeof(void*);
    }

    u64 aligned_size = war_align_up(config->object_size, alignment);

    // Ensure objects are large enough to hold free list node
    if (aligned_size < sizeof(pool_free_node_t))
    {
        aligned_size = sizeof(pool_free_node_t);
    }

    // Calculate optimal objects per block
    u64 objects_per_block = config->objects_per_block;
    if (objects_per_block == 0)
    {
        // Auto-calculate based on object size
        if (aligned_size <= 64)
        {
            objects_per_block = 4096; // Small objects: many per block
        }
        else if (aligned_size <= 256)
        {
            objects_per_block = 1024;
        }
        else if (aligned_size <= 1024)
        {
            objects_per_block = 256;
        }
        else
        {
            objects_per_block = 64; // Large objects: fewer per block
        }
    }

    // Create isolated heap
    mi_heap_t* heap = mi_heap_new();
    if (!heap)
        return NULL;

    // Set heap options
    if (config->eager_commit)
    {
        mi_option_set(mi_option_eager_commit, 1);
    }

    // Allocate pool structure
    pool_allocator_t* pool = mi_heap_calloc(heap, 1, sizeof(pool_allocator_t));
    if (!pool)
    {
        mi_heap_delete(heap);
        return NULL;
    }

    pool->object_size = aligned_size;
    pool->object_alignment = alignment;
    pool->objects_per_block = objects_per_block;
    pool->heap = heap;
    pool->use_thread_local = config->use_thread_local;

#if POOL_DEBUG
    pool->name = config->name ? mi_heap_strdup(heap, config->name) : "unnamed";
    pool->detect_double_free = config->detect_double_free;
    pool->clear_on_free = config->clear_on_alloc;
#endif

    // Pre-allocate initial blocks
    for (u64 i = 0; i < config->initial_blocks; i++)
    {
        pool_block_t* block = pool_block_create(pool);
        if (!block)
        {
            // Clean up on failure
            pool_destroy(pool);
            return NULL;
        }

        if (i == 0)
        {
            pool->active_block = block;
        }
    }

    // If no initial blocks, create one on first allocation
    if (config->initial_blocks == 0)
    {
        pool_block_t* block = pool_block_create(pool);
        if (!block)
        {
            pool_destroy(pool);
            return NULL;
        }
        pool->active_block = block;
    }

    return pool;
}

pool_allocator_t* pool_create(u64 object_size, u64 max_objects)
{
    const pool_config_t config = {.object_size = object_size,
                            .object_alignment = 0,                                  // Auto
                            .objects_per_block = max_objects > 0 ? max_objects : 0, // Auto if 0
                            .initial_blocks = 1,
                            .name = "pool",
                            .use_thread_local = true,
                            .eager_commit = false,
                            .clear_on_alloc = false,
                            .detect_double_free = POOL_DEBUG};
    return pool_create_ex(&config);
}

void pool_destroy(pool_allocator_t* pool)
{
    if (!pool)
        return;

#if POOL_DEBUG
    if (pool->allocated_objects > 0)
    {
        printf("Warning: Pool '%s' destroyed with %zu objects still allocated\n", pool->name, pool->allocated_objects);
    }

    printf("Pool '%s' lifetime stats:\n", pool->name);
    printf("  Total allocations: %zu\n", pool->allocation_count);
    printf("  Total deallocations: %zu\n", pool->deallocation_count);
    printf("  Peak allocated: %zu objects\n", pool->peak_allocated);
    printf("  Block count: %zu\n", pool->block_count);
#endif

    // All blocks are freed when heap is deleted
    mi_heap_delete(pool->heap);
}

WAR_FORCE_INLINE static void* pool_alloc_from_block(const pool_allocator_t* pool, pool_block_t* block)
{
    if (!block->free_list)
        return NULL;

    // Pop from free list
    pool_free_node_t* node = block->free_list;

#if POOL_DEBUG
    if (pool->detect_double_free && node->magic != POOL_FREE_PATTERN)
    {
        assert(!"Pool corruption: Invalid free node magic");
        return NULL;
    }
#endif

    block->free_list = node->next;
    block->objects_in_use++;

    void* ptr = node;

#if POOL_DEBUG
    // Mark as allocated in bitmap
    const u64 index = ((uint8_t*) ptr - block->memory) / pool->object_size;
    const u64 bitmap_index = index / 64;
    const u64 bit_index = index % 64;
    block->allocation_bitmap[bitmap_index] |= 1ULL << bit_index;

    // Clear magic to detect use-after-free
    node->magic = POOL_ALLOC_PATTERN;

    if (pool->clear_on_free)
    {
        memset(ptr, 0, pool->object_size);
    }
#endif

    return ptr;
}

void* pool_alloc(pool_allocator_t* pool)
{
    if (WAR_UNLIKELY(!pool))
        return NULL;

    // Try global free list first (cross-block reuse)
    if (pool->global_free_list)
    {
        pool_free_node_t* node = pool->global_free_list;
        pool->global_free_list = node->next;

        pool->allocated_objects++;
#if POOL_DEBUG
        pool->allocation_count++;
        if (pool->allocated_objects > pool->peak_allocated)
        {
            pool->peak_allocated = pool->allocated_objects;
        }
#endif
        return node;
    }

    // Try active block
    if (pool->active_block)
    {
        void* ptr = pool_alloc_from_block(pool, pool->active_block);
        if (ptr)
        {
            pool->allocated_objects++;
#if POOL_DEBUG
            pool->allocation_count++;
            if (pool->allocated_objects > pool->peak_allocated)
            {
                pool->peak_allocated = pool->allocated_objects;
            }
#endif
            return ptr;
        }
    }

    // Search other blocks with free space
    pool_block_t* block = pool->blocks;
    while (block)
    {
        if (block->free_list)
        {
            void* ptr = pool_alloc_from_block(pool, block);
            if (ptr)
            {
                pool->active_block = block; // Cache for next allocation
                pool->allocated_objects++;
#if POOL_DEBUG
                pool->allocation_count++;
                if (pool->allocated_objects > pool->peak_allocated)
                {
                    pool->peak_allocated = pool->allocated_objects;
                }
#endif
                return ptr;
            }
        }
        block = block->next;
    }

    // Need new block
    pool_block_t* new_block = pool_block_create(pool);
    if (!new_block)
        return NULL;

    pool->active_block = new_block;
    void* ptr = pool_alloc_from_block(pool, new_block);

    if (ptr)
    {
        pool->allocated_objects++;
#if POOL_DEBUG
        pool->allocation_count++;
        if (pool->allocated_objects > pool->peak_allocated)
        {
            pool->peak_allocated = pool->allocated_objects;
        }
#endif
    }

    return ptr;
}

// Fast path for thread-local allocation
WAR_FORCE_INLINE static void* pool_alloc_fast_path(pool_allocator_t* pool)
{
    pool_free_node_t* node = pool->global_free_list;
    if (WAR_LIKELY(node != NULL))
    {
        pool->global_free_list = node->next;
        pool->allocated_objects++;
        return node;
    }
    return pool_alloc(pool);
}

void pool_free(pool_allocator_t* pool, void* ptr)
{
    if (WAR_UNLIKELY(!pool || !ptr))
        return;

#if POOL_DEBUG
    // Find which block owns this pointer
    const pool_block_t* block = pool->blocks;
    bool found = false;

    while (block)
    {
        const uint8_t* start = block->memory;
        const uint8_t* end = start + block->num_objects * pool->object_size;

        if ((uint8_t*) ptr >= start && (uint8_t*) ptr < end)
        {
            // Verify alignment
            const u64 offset = (uint8_t*) ptr - start;
            if (offset % pool->object_size != 0)
            {
                assert(!"Pool corruption: Misaligned pointer");
                return;
            }

            // Check if already free using bitmap
            const u64 index = offset / pool->object_size;
            const u64 bitmap_index = index / 64;
            const u64 bit_index = index % 64;

            if (!(block->allocation_bitmap[bitmap_index] & 1ULL << bit_index))
            {
                assert(!"Double free detected");
                return;
            }

            // Clear bit
            block->allocation_bitmap[bitmap_index] &= ~(1ULL << bit_index);

            found = true;
            break;
        }
        block = block->next;
    }

    if (!found)
    {
        assert(!"Pointer not from this pool");
        return;
    }

    pool->deallocation_count++;
#endif

    // Add to global free list for fast reuse
    pool_free_node_t* node = ptr;

#if POOL_DEBUG
    node->magic = POOL_FREE_PATTERN;
    if (pool->clear_on_free)
    {
        // Clear memory except for free list node
        memset((uint8_t*) ptr + sizeof(pool_free_node_t), 0, pool->object_size - sizeof(pool_free_node_t));
    }
#endif

    node->next = pool->global_free_list;
    pool->global_free_list = node;
    pool->allocated_objects--;
}

void** pool_alloc_bulk(pool_allocator_t* pool, u64 count)
{
    if (!pool || count == 0)
        return NULL;

    // Allocate array for pointers
    void** ptrs = mi_heap_malloc(pool->heap, sizeof(void*) * count);
    if (!ptrs)
        return NULL;

    // Try to allocate from same block for locality
    u64 allocated = 0;

    // First, drain the global free list
    while (allocated < count && pool->global_free_list)
    {
        ptrs[allocated++] = pool_alloc(pool);
    }

    // Then allocate from blocks
    pool_block_t* block = pool->active_block;
    while (allocated < count && block)
    {
        while (allocated < count && block->free_list)
        {
            ptrs[allocated++] = pool_alloc_from_block(pool, block);
        }
        block = block->next;
    }

    // Create new blocks if needed
    while (allocated < count)
    {
        void* ptr = pool_alloc(pool);
        if (!ptr)
        {
            // Rollback on failure
            pool_free_bulk(pool, ptrs, allocated);
            mi_free(ptrs);
            return NULL;
        }
        ptrs[allocated++] = ptr;
    }

    pool->allocated_objects += count;

#if POOL_DEBUG
    pool->allocation_count += count;
    if (pool->allocated_objects > pool->peak_allocated)
    {
        pool->peak_allocated = pool->allocated_objects;
    }
#endif

    return ptrs;
}

void pool_free_bulk(pool_allocator_t* pool, void** ptrs, u64 count)
{
    if (!pool || !ptrs || count == 0)
        return;

    for (u64 i = 0; i < count; i++)
    {
        if (ptrs[i])
        {
            pool_free(pool, ptrs[i]);
        }
    }
}

bool pool_contains(pool_allocator_t* pool, void* ptr)
{
    if (!pool || !ptr)
        return false;

    const pool_block_t* block = pool->blocks;
    while (block)
    {
        const uint8_t* start = block->memory;
        const uint8_t* end = start + block->num_objects * pool->object_size;

        if ((uint8_t*) ptr >= start && (uint8_t*) ptr < end)
        {
            // Verify alignment
            const u64 offset = (uint8_t*) ptr - start;
            return offset % pool->object_size == 0;
        }
        block = block->next;
    }

    return false;
}

u64 pool_get_allocation_size(const pool_allocator_t* pool)
{
    return pool ? pool->object_size : 0;
}

void pool_clear(pool_allocator_t* pool)
{
    if (!pool)
        return;

    // Reset all blocks to fully free
    pool_block_t* block = pool->blocks;
    while (block)
    {
        block->objects_in_use = 0;

        // Rebuild free list for this block
        pool_free_node_t* prev = NULL;
        const u64 object_size = pool->object_size;

        for (u64 i = 0; i < block->num_objects; i++)
        {
            pool_free_node_t* node = (pool_free_node_t*) (block->memory + i * object_size);
            node->next = prev;
#if POOL_DEBUG
            node->magic = POOL_FREE_PATTERN;
#endif
            prev = node;
        }
        block->free_list = prev;

#if POOL_DEBUG
        // Clear allocation bitmap
        const u64 bitmap_size = (block->num_objects + 63) / 64 * sizeof(uint64_t);
        memset(block->allocation_bitmap, 0, bitmap_size);
#endif

        block = block->next;
    }

    // Clear global free list
    pool->global_free_list = NULL;
    pool->allocated_objects = 0;

    // Reset active block
    pool->active_block = pool->blocks;
}


static void pool_stats_print_callback(const char* msg, const void* arg)
{
    (void) arg; // Unused
    printf("%s", msg);
}

void pool_print_stats(pool_allocator_t* pool)
{
    if (!pool)
        return;

    printf("Pool Allocator Statistics:\n");
#if POOL_DEBUG
    printf("  Name: %s\n", pool->name);
#endif
    printf("  Object size: %zu bytes\n", pool->object_size);
    printf("  Objects per block: %zu\n", pool->objects_per_block);
    printf("  Total blocks: %zu\n", pool->block_count);
    printf("  Total objects: %zu\n", pool->total_objects);
    printf("  Allocated objects: %zu\n", pool->allocated_objects);
    printf("  Free objects: %zu\n", pool->total_objects - pool->allocated_objects);
    printf("  Memory usage: %zu KB\n", pool->block_count * pool->objects_per_block * pool->object_size / 1024);

#if POOL_DEBUG
    printf("  Peak allocated: %zu objects\n", pool->peak_allocated);
    printf("  Total allocations: %zu\n", pool->allocation_count);
    printf("  Total deallocations: %zu\n", pool->deallocation_count);

    // Per-block statistics
    printf("\n  Per-block usage:\n");
    const pool_block_t* block = pool->blocks;
    int block_num = 0;
    while (block)
    {
        printf("    Block %d: %zu/%zu objects (%.1f%% full)\n", block_num++, block->objects_in_use, block->num_objects,
               (double) block->objects_in_use * 100.0 / (double) block->num_objects);
        block = block->next;
    }
#endif

    // Print mimalloc heap statistics
    mi_heap_collect(pool->heap, false);
    mi_stats_print_out(pool_stats_print_callback, pool);
}