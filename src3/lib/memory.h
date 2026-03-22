#pragma once

#include <stdlib.h>
#include <stdint.h>

typedef struct WK_Allocator
{
    void* (*alloc_fn)(size_t size, void* user_data);
    void (*free_fn)(void* ptr, void* user_data);
    void* (*calloc_fn)(size_t size, size_t count, void* user_data);
    void* (*realloc_fn)(void* ptr, size_t size, void* user_data);
    void* user_data;
};

typedef struct WK_Arena
{
    int       alignment;
    int       block_size;
    uint8_t*  ptr;
    uint8_t*  end;
    int       block_index;
    uint8_t** blocks;
} CF_Arena;