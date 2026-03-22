#include <core/arena.h>

#include <assert.h>
#include <mimalloc.h>
#include <stdio.h>

// Add required headers for page size detection
#ifdef _WIN32
#    include <windows.h>
#else
#    include <unistd.h>
#endif

struct arena_block
{
    struct arena_block* next;
    u64 size;
    u64 used;
    RG_ALIGN(ARENA_DEFAULT_ALIGNMENT) u8 data[];
};

struct arena
{
    arena_block_t* current;
    arena_block_t* first;
    u64 block_size;
    u64 total_allocated;
    u64 total_used;

    // mimalloc heap for memory isolation
    mi_heap_t* heap;

    // Thread-local heap for better performance
    bool use_thread_local;

#if ARENA_DEBUG
    u64 allocation_count;
    u64 peak_usage;
    u64 block_count;
    const char* name;
    mi_heap_t* debug_heap; // Separate heap for debug metadata
#endif
};

static arena_block_t* arena_block_create(arena_t* arena, const u64 size)
{
    const u64 total_size = sizeof(arena_block_t) + size;

    // Allocate from mimalloc heap with optimal alignment
    arena_block_t* block = mi_heap_malloc_aligned(arena->heap, total_size, ARENA_DEFAULT_ALIGNMENT);

    if (!block)
        return NULL;

    block->next = NULL;
    block->size = size;
    block->used = 0;

#if ARENA_DEBUG
    arena->block_count++;
#endif

    return block;
}

arena_t* arena_create_ex(const arena_config_t* config)
{
    u64 initial_size = config->initial_size;
    if (initial_size < 4096)
        initial_size = 4096;
    // Use standard page size - mimalloc handles page alignment internally
    u64 page_size;
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    page_size = si.dwPageSize;
#else
    page_size = sysconf(_SC_PAGESIZE);
#endif
    initial_size = align_up(initial_size, page_size);

    // Create isolated heap for this arena
    mi_heap_t* heap = mi_heap_new();
    if (!heap)
        return NULL;

    // Configure global options before creating heap
    // Note: These affect all heaps, not just this one
    if (config->eager_commit)
    {
        mi_option_set(mi_option_eager_commit, 1);
    }
    if (config->allow_large_pages)
    {
        mi_option_set(mi_option_large_os_pages, 1);
    }

    // Allocate arena struct from the heap
    arena_t* arena = mi_heap_calloc(heap, 1, sizeof(arena_t));
    if (!arena)
    {
        mi_heap_delete(heap);
        return NULL;
    }

    arena->heap = heap;
    arena->use_thread_local = config->use_thread_local;
    arena->block_size = initial_size;

    // Create first block
    arena->first = arena->current = arena_block_create(arena, initial_size);
    if (!arena->first)
    {
        mi_heap_delete(heap);
        return NULL;
    }

    arena->total_allocated = initial_size;
    arena->total_used = 0;

#if ARENA_DEBUG
    arena->allocation_count = 0;
    arena->peak_usage = 0;
    arena->block_count = 0;
    arena->name = config->name ? mi_heap_strdup(heap, config->name) : "unnamed";
    arena->debug_heap = mi_heap_new(); // Separate heap for debug data
#endif

    return arena;
}

arena_t* arena_create(const u64 initial_size, const char* name)
{
    const arena_config_t config = {.initial_size = initial_size,
                             .name = name,
                             .use_thread_local = true,
                             .eager_commit = false,
                             .allow_large_pages = false,
                             .max_size = 0};
    return arena_create_ex(&config);
}

void arena_destroy(arena_t* arena)
{
    if (!arena)
        return;

#if ARENA_DEBUG && ARENA_ENABLE_STATS
    if (arena->peak_usage > 0)
    {
        printf("Arena '%s' final stats:\n", arena->name);
        printf("  Peak usage: %zu bytes\n", arena->peak_usage);
        printf("  Total allocated: %zu bytes\n", arena->total_allocated);
        printf("  Block count: %zu\n", arena->block_count);
        printf("  Allocation count: %zu\n", arena->allocation_count);
    }

    mi_heap_delete(arena->debug_heap);
#endif

    // Delete the heap - this frees all blocks at once!
    mi_heap_delete(arena->heap);
}

void* arena_alloc_aligned(arena_t* arena, const u64 size, u64 alignment)
{
    if (size == 0)
        return NULL;
    if (alignment < 1)
        alignment = 1;

    assert((alignment & alignment - 1) == 0); // Power of 2

    // For very large allocations, allocate directly from heap
    if (size > arena->block_size / 4)
    {
        void* ptr = mi_heap_malloc_aligned(arena->heap, size, alignment);
#if ARENA_DEBUG
        if (ptr)
        {
            arena->allocation_count++;
            arena->total_used += size;
            arena->total_allocated += size;
            if (arena->total_used > arena->peak_usage)
            {
                arena->peak_usage = arena->total_used;
            }
        }
#endif
        return ptr;
    }

    arena_block_t* block = arena->current;
    uintptr_t current_ptr = (uintptr_t) (block->data + block->used);
    uintptr_t aligned_ptr = align_ptr(current_ptr, alignment);
    u64 padding = aligned_ptr - current_ptr;
    u64 total_size = padding + size;

    // Check if allocation fits in current block
    if (block->used + total_size > block->size)
    {
        // Try to find space in existing blocks
        arena_block_t* candidate = arena->first;
        while (candidate && candidate != block)
        {
            current_ptr = (uintptr_t) (candidate->data + candidate->used);
            aligned_ptr = align_ptr(current_ptr, alignment);
            padding = aligned_ptr - current_ptr;
            total_size = padding + size;

            if (candidate->used + total_size <= candidate->size)
            {
                arena->current = candidate;
                block = candidate;
                goto allocate;
            }
            candidate = candidate->next;
        }

        // Need new block - double size for growth
        u64 new_block_size = arena->block_size;
        while (new_block_size < total_size)
        {
            new_block_size *= 2;
        }

        // Limit growth to avoid runaway allocation
        if (new_block_size > arena->block_size * 8)
        {
            new_block_size = arena->block_size * 8;
            if (new_block_size < total_size)
            {
                new_block_size = total_size;
            }
        }

        arena_block_t* new_block = arena_block_create(arena, new_block_size);
        if (!new_block)
            return NULL;

        block->next = new_block;
        arena->current = new_block;
        arena->total_allocated += new_block_size;
        block = new_block;

        // Recalculate for new block
        current_ptr = (uintptr_t) block->data;
        aligned_ptr = align_ptr(current_ptr, alignment);
        padding = aligned_ptr - current_ptr;
        total_size = padding + size;
    }

allocate:
    block->used += total_size;
    arena->total_used += size;

#if ARENA_DEBUG
    arena->allocation_count++;
    if (arena->total_used > arena->peak_usage)
    {
        arena->peak_usage = arena->total_used;
    }
#endif

    return (void*) aligned_ptr;
}

void* arena_alloc(arena_t* arena, const u64 size)
{
    return arena_alloc_aligned(arena, size, ARENA_DEFAULT_ALIGNMENT);
}

void arena_reset(arena_t* arena)
{
    // Keep only the first block, free the rest
    arena_block_t* block = arena->first->next;
    arena->first->next = NULL;

    // mimalloc makes this efficient - bulk free
    while (block)
    {
        arena_block_t* next = block->next;
        mi_free(block);
        block = next;
#if ARENA_DEBUG
        arena->block_count--;
#endif
    }

    // Reset first block
    arena->first->used = 0;
    arena->current = arena->first;
    arena->total_used = 0;
    arena->total_allocated = arena->block_size;

#if ARENA_DEBUG
    arena->allocation_count = 0;
#endif
}

arena_mark_t arena_mark(const arena_t* arena)
{
    const arena_mark_t mark = {.block = arena->current, .used = arena->current->used, .total_used = arena->total_used};
    return mark;
}

void arena_restore(arena_t* arena, const arena_mark_t mark)
{
    // Free blocks allocated after the mark
    arena_block_t* block = mark.block->next;
    mark.block->next = NULL;

    while (block)
    {
        arena_block_t* next = block->next;
        mi_free(block);
        arena->total_allocated -= block->size;
        block = next;
#if ARENA_DEBUG
        arena->block_count--;
#endif
    }

    // Restore the marked block state
    mark.block->used = mark.used;
    arena->current = mark.block;
    arena->total_used = mark.total_used;
}

void* arena_calloc(arena_t* arena, const u64 count, const u64 size)
{
    const u64 total = count * size;
    void* ptr = arena_alloc(arena, total);
    if (ptr)
    {
        memset(ptr, 0, total);
    }
    return ptr;
}

void* arena_realloc(arena_t* arena, void* ptr, const u64 old_size, const u64 new_size)
{
    if (!ptr)
        return arena_alloc(arena, new_size);
    if (new_size == 0)
        return NULL;
    if (new_size <= old_size)
        return ptr;

    // Check if we can extend in place (only if it's the last allocation)
    arena_block_t* block = arena->current;
    const u8* ptr_bytes = ptr;
    const u8* block_end = block->data + block->used;

    if (ptr_bytes + old_size == block_end)
    {
        const u64 extra = new_size - old_size;
        if (block->used + extra <= block->size)
        {
            block->used += extra;
            arena->total_used += extra;
            return ptr;
        }
    }

    // Need to allocate new memory
    void* new_ptr = arena_alloc(arena, new_size);
    if (new_ptr)
    {
        memcpy(new_ptr, ptr, old_size);
    }
    return new_ptr;
}

char* arena_strdup(arena_t* arena, const char* str)
{
    if (!str)
        return NULL;
    const u64 len = strlen(str) + 1;
    char* copy = arena_alloc(arena, len);
    if (copy)
    {
        memcpy(copy, str, len);
    }
    return copy;
}

char* arena_strndup(arena_t* arena, const char* str, const u64 n)
{
    if (!str)
        return NULL;
    const u64 len = strnlen(str, n);
    char* copy = arena_alloc(arena, len + 1);
    if (copy)
    {
        memcpy(copy, str, len);
        copy[len] = '\0';
    }
    return copy;
}

static void arena_stats_print_callback(const char* msg, void* arg)
{
    (void) arg; // Unused
    printf("%s", msg);
}

void arena_print_stats(arena_t* arena)
{
#if ARENA_DEBUG && ARENA_ENABLE_STATS
    printf("Arena '%s' statistics:\n", arena->name);
    printf("  Current usage: %zu bytes\n", arena->total_used);
    printf("  Peak usage: %zu bytes\n", arena->peak_usage);
    printf("  Total allocated: %zu bytes\n", arena->total_allocated);
    printf("  Block count: %zu\n", arena->block_count);
    printf("  Allocation count: %zu\n", arena->allocation_count);
    printf("  Fragmentation: %.2f%%\n", arena->total_allocated > 0 ? (1.0 - (double) arena->total_used / (double) arena->total_allocated) * 100 : 0);

    // Print mimalloc heap statistics
    mi_heap_collect(arena->heap, false);
    mi_stats_print_out(arena_stats_print_callback, arena);
#endif
}