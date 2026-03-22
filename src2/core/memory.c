#include <core/memory.h>

#include <mimalloc.h>

void* wc_malloc(const size_t size)
{
    return mi_malloc(size);
}

void* wc_calloc(const size_t count, const size_t size)
{
    return mi_calloc(count, size);
}

void* wc_realloc(void* const memory, const size_t size)
{
    return mi_realloc(memory, size);
}

void wc_free(void* const memory)
{
    mi_free(memory);
}

void* wc_aligned_alloc(const size_t size, const size_t alignment)
{
    return mi_malloc_aligned(size, alignment);
}

void wc_aligned_free(void* memory, const size_t alignment)
{
    mi_free_aligned(memory, alignment);
}
