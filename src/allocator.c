#include "allocator.h"
#include "assert.h"
#include <stdlib.h>
#include <string.h>

#define DEFAULT_ALIGNMENT (2 * sizeof(void*))

static inline bool isPowerOfTwo(uintptr_t x)
{
    return (x & (x - 1)) == 0;
}

static inline uintptr_t alignForward(uintptr_t ptr, size_t align)
{
    WB_ASSERT(isPowerOfTwo(align));

    uintptr_t p = ptr;
    uintptr_t a = (uintptr_t) align;
    uintptr_t modulo = p & (a - 1);

    if (modulo != 0)
        p += a - modulo;

    return p;
}

void wbInitTempAllocator(WbTempAllocator* allocator, void* buffer, uint32_t buffer_size)
{
    WB_ASSERT(allocator != NULL);
    WB_ASSERT(buffer != NULL);
    WB_ASSERT(buffer_size > 0);

    allocator->buffer = buffer;
    allocator->size = buffer_size;
    allocator->offset = 0;
}

void wbResetTempAlloc(WbTempAllocator* allocator)
{
    WB_ASSERT(allocator != NULL);

    allocator->offset = 0;
}

void* wbTempAllocAlign(WbTempAllocator* allocator, uint32_t size, uint32_t alignment)
{
    WB_ASSERT(allocator != NULL);
    WB_ASSERT(allocator->buffer != NULL);
    WB_ASSERT(size > 0);
    WB_ASSERT(alignment > 0);

    uintptr_t current = (uintptr_t) allocator->buffer + (uintptr_t) allocator->offset;
    uintptr_t offset = alignForward(current, alignment);
    offset -= (uintptr_t) allocator->buffer;

    uint32_t new_offset = offset + size;
    if (new_offset > allocator->size)
    {
        return NULL;
    }

    void* pointer = &allocator->buffer[offset];
    allocator->offset = new_offset;

    memset(pointer, 0, size);
    return pointer;
}

void* wbTempAlloc(WbTempAllocator* allocator, uint32_t size)
{
    return wbTempAllocAlign(allocator, size, DEFAULT_ALIGNMENT);
}
