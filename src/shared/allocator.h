#pragma once

#include "types.h"

void wbInitTempAllocator(WbTempAllocator* allocator, void* buffer, uint32_t buffer_size);
void wbResetTempAlloc(WbTempAllocator* allocator);
void* wbTempAlloc(WbTempAllocator* allocator, uint32_t size);
void* wbTempAllocAlign(WbTempAllocator* allocator, uint32_t size, uint32_t alignment);
