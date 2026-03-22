#pragma once

#include <core/types.h>

// Configuration
#ifndef ARENA_DEFAULT_ALIGNMENT
#    define ARENA_DEFAULT_ALIGNMENT 16
#endif

#ifndef ARENA_DEBUG
#    ifdef NDEBUG
#        define ARENA_DEBUG 0
#    else
#        define ARENA_DEBUG 1
#    endif
#endif

// Enable mimalloc stats in debug builds
#if ARENA_DEBUG
#    define ARENA_ENABLE_STATS 1
#else
#    define ARENA_ENABLE_STATS 0
#endif

typedef struct arena arena_t;
typedef struct arena_block arena_block_t;

typedef struct arena_mark
{
    arena_block_t* block;
    u64 used;
    u64 total_used;
} arena_mark_t;

typedef struct arena_config
{
    u64 initial_size;
    const char* name;
    bool use_thread_local;  // Use thread-local heap for better MT performance
    bool eager_commit;      // Commit memory pages immediately
    bool allow_large_pages; // Use large OS pages if available
    u64 max_size;        // Maximum arena size (0 = unlimited)
} arena_config_t;

// Core functions
arena_t* arena_create(u64 initial_size, const char* name);
arena_t* arena_create_ex(const arena_config_t* config);
void arena_destroy(arena_t* arena);
void* arena_alloc(arena_t* arena, u64 size);
void* arena_alloc_aligned(arena_t* arena, u64 size, u64 alignment);
void arena_reset(arena_t* arena);
arena_mark_t arena_mark(const arena_t* arena);
void arena_restore(arena_t* arena, arena_mark_t mark);

// Utility functions
void* arena_calloc(arena_t* arena, u64 count, u64 size);
void* arena_realloc(arena_t* arena, void* ptr, u64 old_size, u64 new_size);
char* arena_strdup(arena_t* arena, const char* str);
char* arena_strndup(arena_t* arena, const char* str, u64 n);
void arena_print_stats(arena_t* arena);

// Type-safe allocation macros
#define ARENA_NEW(arena, type) ((type*) arena_alloc_aligned(arena, sizeof(type), _Alignof(type)))

#define ARENA_NEW_ARRAY(arena, type, count) ((type*) arena_alloc_aligned(arena, sizeof(type) * (count), _Alignof(type)))

#define ARENA_NEW_ZERO(arena, type) ((type*) arena_calloc(arena, 1, sizeof(type)))

#define ARENA_NEW_ARRAY_ZERO(arena, type, count) ((type*) arena_calloc(arena, count, sizeof(type)))
