#pragma once

#include <stdint.h>

typedef struct b_allocator_t b_allocator_t;

void b_init_allocator(b_allocator_t* allocator, void* buffer, uint32_t buffer_size);
void b_reset_allocator(b_allocator_t* allocator);
void* b_alloc(b_allocator_t* allocator, uint32_t size);
void* b_alloc_align(b_allocator_t* allocator, uint32_t size, uint32_t alignment);
