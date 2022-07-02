#pragma once

#include "types.h"

typedef struct
{
	const void* data;
	void* map_object;
} wb_memory_mapping;

u8* wb_memory_virtual_alloc(u64 size);
void wb_memory_virtual_free(u8* memory);

void wb_memory_map(const char* path, wb_memory_mapping* mapping);
void wb_memory_unmap(wb_memory_mapping* mapping);