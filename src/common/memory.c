#include "memory.h"
#ifdef _WIN64
#include "windows_io.h"
#include "windows_file.h"
#include "windows_misc.h"
#endif
#include <assert.h>

u8* wb_memory_virtual_alloc(u64 size)
{
    u8* memory = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
    assert(memory);
    return memory;
}

void wb_memory_virtual_free(u8* memory)
{
    VirtualFree(memory, 0, MEM_RELEASE);
}

void wb_memory_map(const char* path, wb_memory_mapping* mapping)
{
#ifdef _WIN64
    wchar_t buffer[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, buffer, MAX_PATH);

    HANDLE file = CreateFileW(
        buffer,
        FILE_GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    assert(file != INVALID_HANDLE_VALUE);

    mapping->map_object = CreateFileMappingW(
        file,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL);
    assert(mapping->map_object != NULL);

    mapping->data = MapViewOfFile(
        mapping->map_object,
        FILE_MAP_READ,
        0,
        0,
        0);
    assert(mapping->data != NULL);

    CloseHandle(file);
#endif
}

void wb_memory_unmap(wb_memory_mapping* mapping)
{
#ifdef _WIN64
    if (mapping->data != NULL)
        UnmapViewOfFile(mapping->data);
    if (mapping->map_object != NULL)
        CloseHandle(mapping->map_object);
#endif
}

//#define DEFAULT_ALIGNMENT (2 * sizeof(void*))
//
//static inline bool isPowerOfTwo(uintptr_t x)
//{
//    return (x & (x - 1)) == 0;
//}
//
//static inline uintptr_t alignForward(uintptr_t ptr, size_t align)
//{
//    assert(isPowerOfTwo(align));
//
//    uintptr_t p = ptr;
//    uintptr_t a = (uintptr_t) align;
//    uintptr_t modulo = p & (a - 1);
//
//    if (modulo != 0)
//        p += a - modulo;
//
//    return p;
//}

//void b_init_allocator(allocator_t* allocator, void* buffer, u32 buffer_size)
//{
//    assert(allocator != NULL);
//    assert(buffer != NULL);
//    assert(buffer_size > 0);
//
//    allocator->buffer = buffer;
//    allocator->source_size = buffer_size;
//    allocator->offset = 0;
//}
//
//void b_reset_allocator(allocator_t* allocator)
//{
//    assert(allocator != NULL);
//
//    allocator->offset = 0;
//}
//
//void* b_alloc_align(allocator_t* allocator, u32 source_size, u32 alignment)
//{
//    assert(allocator != NULL);
//    assert(allocator->buffer != NULL);
//    assert(source_size > 0);
//    assert(alignment > 0);
//
//    uintptr_t current = (uintptr_t) allocator->buffer + (uintptr_t) allocator->offset;
//    uintptr_t offset = alignForward(current, alignment);
//    offset -= (uintptr_t) allocator->buffer;
//
//    u32 new_offset = offset + source_size;
//    if (new_offset > allocator->source_size)
//    {
//        return NULL;
//    }
//
//    void* pointer = &allocator->buffer[offset];
//    allocator->offset = new_offset;
//
//    memset(pointer, 0, source_size);
//    return pointer;
//}

//void* b_alloc(allocator_t* allocator, u32 source_size)
//{
//    return alloc_align(allocator, source_size, DEFAULT_ALIGNMENT);
//}
