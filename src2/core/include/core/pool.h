#pragma once

#include "common.h"

// Configuration
#ifndef POOL_DEBUG
    #ifdef NDEBUG
        #define POOL_DEBUG 0
    #else
        #define POOL_DEBUG 1
    #endif
#endif

typedef struct pool_allocator pool_allocator_t;
typedef struct pool_block pool_block_t;

// Configuration structure
typedef struct {
    u64 object_size;         // Size of each object
    u64 object_alignment;    // Alignment (0 = default)
    u64 objects_per_block;   // Objects per block (0 = auto)
    u64 initial_blocks;      // Pre-allocate blocks
    const char* name;           // Debug name
    bool use_thread_local;      // Thread-local allocation
    bool eager_commit;          // Commit memory immediately
    bool clear_on_alloc;        // Zero memory on allocation
    bool detect_double_free;    // Enable double-free detection
} pool_config_t;

// Core API
pool_allocator_t* pool_create(u64 object_size, u64 max_objects);
pool_allocator_t* pool_create_ex(const pool_config_t* config);
void pool_destroy(pool_allocator_t* pool);

// Allocation/deallocation
void* pool_alloc(pool_allocator_t* pool);
void pool_free(pool_allocator_t* pool, void* ptr);

// Bulk operations
void** pool_alloc_bulk(pool_allocator_t* pool, u64 count);
void pool_free_bulk(pool_allocator_t* pool, void** ptrs, u64 count);

// Utilities
bool pool_contains(pool_allocator_t* pool, void* ptr);
u64 pool_get_allocation_size(const pool_allocator_t* pool);
void pool_clear(pool_allocator_t* pool);
void pool_print_stats(pool_allocator_t* pool);

// Thread-local fast path
#define POOL_ALLOC_FAST(pool) \
((pool)->use_thread_local && (pool)->global_free_list ? \
pool_alloc_fast_path(pool) : pool_alloc(pool))

// Type-safe allocation macros
#define POOL_NEW(pool, type) \
((type*)pool_alloc(pool))

#define POOL_DELETE(pool, ptr) \
pool_free(pool, ptr)