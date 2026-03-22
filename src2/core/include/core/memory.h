#pragma once

#include <stdlib.h>

//-------------------------------------------------------------------------------------------------
// General allocator

void* wc_malloc(size_t size);
void* wc_calloc(size_t count, size_t size);
void* wc_realloc(void* memory, size_t size);
void wc_free(void* memory);

void* wc_aligned_alloc(size_t size, size_t alignment);
void wc_aligned_free(void* memory, size_t alignment);

//-------------------------------------------------------------------------------------------------
// Arena allocator
