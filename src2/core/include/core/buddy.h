/**
 * @file buddy.h
 * @brief Buddy Allocator - Binary Tree Memory Management System
 * 
 * ALGORITHM OVERVIEW:
 * ==================
 * The buddy allocator implements a binary tree-based memory management system that
 * maintains free blocks in power-of-2 sizes. Memory is allocated and freed in
 * blocks whose sizes are powers of 2, enabling efficient coalescing and splitting.
 * 
 * Key Algorithm Properties:
 * - All block sizes are powers of 2 (2^order * min_block_size)
 * - Each block has a "buddy" at the same level that can be merged with it
 * - Splitting: Large blocks are recursively split until reaching required size
 * - Coalescing: Adjacent free buddy blocks are merged to reduce fragmentation
 * - Fast allocation/deallocation: O(log n) worst case, often O(1) in practice
 * 
 * CONFIGURATION OPTIONS:
 * =====================
 * - BUDDY_DEFAULT_MIN_BLOCK: Minimum allocatable block size (default: 64 bytes)
 * - BUDDY_DEFAULT_ALIGNMENT: Default memory alignment (default: 16 bytes)
 * - BUDDY_MAX_ORDER: Maximum order for blocks (default: 20, supports up to 1MB)
 * - BUDDY_DEBUG: Enable debug features and validation (auto-detected from NDEBUG)
 * - BUDDY_ENABLE_TRACKING: Enable allocation tracking (debug builds only)
 * - BUDDY_ENABLE_STATS: Enable detailed statistics collection (debug builds only)
 * 
 * Configuration Structure Options:
 * - min_block_size: Minimum block size (must be power of 2)
 * - max_heap_size: Maximum total heap size
 * - eager_commit: Commit memory pages immediately vs on-demand
 * - use_thread_local: Use thread-local heaps for better MT performance
 * - allow_large_pages: Enable OS large page support if available
 * - clear_on_alloc: Zero memory on allocation (debug feature)
 * - coalesce_on_free: Immediately coalesce adjacent blocks on free
 * 
 * COMPLEXITY GUARANTEES:
 * ======================
 * - Allocation: O(log n) worst case, O(1) typical case
 * - Deallocation: O(log n) worst case due to coalescing
 * - Memory overhead: ~2% for metadata and free list management
 * - Fragmentation: Bounded by buddy algorithm (max 25% internal fragmentation)
 * - Space complexity: O(log n) for free list structures
 * 
 * Performance Characteristics:
 * - Best case: O(1) when appropriately sized blocks are available
 * - Worst case: O(log n) when recursive splitting/coalescing is required
 * - Fragmentation is predictable and bounded unlike general-purpose allocators
 * 
 * THREAD SAFETY:
 * ==============
 * - DEFAULT: NOT thread-safe - single-threaded use only
 * - With use_thread_local=true: Each thread gets isolated heap (thread-safe)
 * - Manual synchronization required for shared buddy_t instances
 * - Underlying mimalloc heaps provide thread-local isolation when enabled
 * 
 * Thread Safety Notes:
 * - buddy_create/buddy_destroy: Not thread-safe, must be called from main thread
 * - buddy_alloc/buddy_free: Thread-safe only with use_thread_local=true
 * - Statistics functions: Not thread-safe, use external synchronization
 * - Configuration changes: Not thread-safe, set during initialization only
 * 
 * INTEGRATION NOTES:
 * ==================
 * - Built on top of mimalloc for underlying memory management
 * - Integrates with war_align_up() and other common.h utilities
 * - Uses WAR_ALIGN, WAR_FORCE_INLINE, and other portability macros
 * - Compatible with existing arena and pool allocators in the codebase
 * - Type-safe allocation macros provide C++ style new/delete semantics
 * 
 * USAGE PATTERNS:
 * ===============
 * 1. Simple Usage:
 *    buddy_t* buddy = buddy_create(1024*1024, "game_allocator");
 *    void* ptr = buddy_alloc(buddy, 256);
 *    buddy_free(buddy, ptr);
 *    buddy_destroy(buddy);
 * 
 * 2. Type-safe Usage:
 *    GameEntity* entity = BUDDY_NEW(buddy, GameEntity);
 *    GameEntity* entities = BUDDY_NEW_ARRAY(buddy, GameEntity, 100);
 *    BUDDY_DELETE(buddy, entity);
 * 
 * 3. Custom Configuration:
 *    buddy_config_t config = {
 *        .max_heap_size = 16*1024*1024,
 *        .min_block_size = 128,
 *        .use_thread_local = true,
 *        .coalesce_on_free = true,
 *        .name = "renderer_buddy"
 *    };
 *    buddy_t* buddy = buddy_create_ex(&config);
 * 
 * @author Warcry Engine Team
 * @version 1.0
 * @date 2024
 */

#pragma once

#include "../../game/engine/common.h"

#include <stdbool.h>

// Configuration
#ifndef BUDDY_DEFAULT_MIN_BLOCK
#    define BUDDY_DEFAULT_MIN_BLOCK 64
#endif

#ifndef BUDDY_DEFAULT_ALIGNMENT
#    define BUDDY_DEFAULT_ALIGNMENT 16  // 16-byte default alignment like arena
#endif

#ifndef BUDDY_MAX_ORDER
#    define BUDDY_MAX_ORDER 20  // Support up to 1MB blocks
#endif

#ifndef BUDDY_DEBUG
#    ifdef NDEBUG
#        define BUDDY_DEBUG 0
#    else
#        define BUDDY_DEBUG 1
#    endif
#endif

// Enable allocation tracking in debug builds
#if BUDDY_DEBUG
#    define BUDDY_ENABLE_TRACKING 1
#else
#    define BUDDY_ENABLE_TRACKING 0
#endif

// Enable mimalloc stats in debug builds
#if BUDDY_DEBUG
#    define BUDDY_ENABLE_STATS 1
#else
#    define BUDDY_ENABLE_STATS 0
#endif

// Forward declarations
typedef struct buddy buddy_t;
typedef struct buddy_node buddy_node_t;

// Configuration structure
typedef struct buddy_config
{
    u64 min_block_size;      // Minimum block size (must be power of 2)
    u64 max_heap_size;       // Maximum heap size
    const char* name;        // Debug name for the allocator
    bool eager_commit;       // Commit memory pages immediately
    bool use_thread_local;   // Use thread-local heap for better MT performance
    bool allow_large_pages;  // Use large OS pages if available
    bool clear_on_alloc;     // Zero memory on allocation
    bool coalesce_on_free;   // Coalesce adjacent free blocks immediately
} buddy_config_t;

// Core API functions

/**
 * Create a buddy allocator with default configuration.
 * @param max_heap_size Maximum size of the heap (must be power of 2)
 * @param name Debug name for the allocator
 * @return New buddy allocator instance, or NULL on failure
 */
buddy_t* buddy_create(u64 max_heap_size, const char* name);

/**
 * Create a buddy allocator with custom configuration.
 * @param config Configuration structure
 * @return New buddy allocator instance, or NULL on failure
 */
buddy_t* buddy_create_ex(const buddy_config_t* config);

/**
 * Destroy a buddy allocator and free all associated memory.
 * @param buddy Allocator to destroy
 */
void buddy_destroy(buddy_t* buddy);

/**
 * Allocate memory from the buddy allocator.
 * @param buddy Allocator instance
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
void* buddy_alloc(buddy_t* buddy, u64 size);

/**
 * Allocate aligned memory from the buddy allocator.
 * @param buddy Allocator instance
 * @param size Number of bytes to allocate
 * @param alignment Required alignment (must be power of 2)
 * @return Pointer to allocated memory, or NULL on failure
 */
void* buddy_alloc_aligned(buddy_t* buddy, u64 size, u64 alignment);

/**
 * Free memory allocated by the buddy allocator.
 * @param buddy Allocator instance
 * @param ptr Pointer to memory to free (must be from this allocator)
 */
void buddy_free(buddy_t* buddy, void* ptr);

/**
 * Reset the buddy allocator, freeing all allocated memory.
 * @param buddy Allocator instance
 */
void buddy_reset(buddy_t* buddy);

/**
 * Print allocation statistics and debug information.
 * @param buddy Allocator instance
 */
void buddy_print_stats(buddy_t* buddy);

/**
 * Validate the internal consistency of the buddy allocator.
 * @param buddy Allocator instance
 * @return true if allocator is consistent, false otherwise
 */
bool buddy_validate(buddy_t* buddy);

// Utility functions

/**
 * Allocate and zero-initialize memory.
 * @param buddy Allocator instance
 * @param count Number of elements
 * @param size Size of each element
 * @return Pointer to zeroed memory, or NULL on failure
 */
void* buddy_calloc(buddy_t* buddy, u64 count, u64 size);

/**
 * Get the size of the block containing the given pointer.
 * @param buddy Allocator instance
 * @param ptr Pointer to allocated memory
 * @return Size of the block, or 0 if pointer is invalid
 */
u64 buddy_get_block_size(buddy_t* buddy, void* ptr);

/**
 * Check if a pointer was allocated by this buddy allocator.
 * @param buddy Allocator instance
 * @param ptr Pointer to check
 * @return true if pointer is from this allocator, false otherwise
 */
bool buddy_contains(buddy_t* buddy, void* ptr);

/**
 * Get the total amount of allocated memory for external monitoring.
 * @param buddy Allocator instance
 * @return Total bytes currently allocated
 */
u64 buddy_get_allocated_size(buddy_t* buddy);

/**
 * Dump the current layout of free lists for debugging (optional).
 * @param buddy Allocator instance
 */
void buddy_dump_layout(buddy_t* buddy);

// Type-safe allocation macros

/**
 * Allocate memory for a single object of the given type.
 * Memory is properly aligned for the type.
 */
#define BUDDY_NEW(buddy, type) \
    ((type*) buddy_alloc_aligned(buddy, sizeof(type), _Alignof(type)))

/**
 * Allocate memory for an array of objects of the given type.
 * Memory is properly aligned for the type.
 */
#define BUDDY_NEW_ARRAY(buddy, type, count) \
    ((type*) buddy_alloc_aligned(buddy, sizeof(type) * (count), _Alignof(type)))

/**
 * Allocate and zero-initialize memory for a single object of the given type.
 */
#define BUDDY_NEW_ZERO(buddy, type) \
    ((type*) buddy_calloc(buddy, 1, sizeof(type)))

/**
 * Allocate and zero-initialize memory for an array of objects of the given type.
 */
#define BUDDY_NEW_ARRAY_ZERO(buddy, type, count) \
    ((type*) buddy_calloc(buddy, count, sizeof(type)))

/**
 * Free memory allocated with BUDDY_NEW or related macros.
 */
#define BUDDY_DELETE(buddy, ptr) \
    buddy_free(buddy, ptr)
