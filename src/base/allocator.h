#pragma once

#include <stdint.h>

typedef struct allocator_t allocator_t;

extern allocator_t* system_allocator;

void allocator_init(allocator_t* allocator, void* buffer, uint32_t buffer_size);
void reset_allocator(allocator_t* allocator);
void* alloc(allocator_t* allocator, uint32_t size);
void* alloc_align(allocator_t* allocator, uint32_t size, uint32_t alignment);
