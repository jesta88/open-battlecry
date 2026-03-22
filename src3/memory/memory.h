#pragma once

#include "types.h"

#define WK_MINIMUM_STACK_ALIGNMENT 16u
#define WK_CACHE_LINE_SIZE 64u

typedef struct {
    char* buffer;
    usize size;
} wk_string;

typedef struct {
    void* (*alloc)(usize size, usize align);
    void (*free)(void* ptr);
    const char* name;
} wk_allocator;

const wk_allocator* g_system_allocator;
