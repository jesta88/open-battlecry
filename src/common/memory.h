#pragma once

#include "types.h"

typedef struct
{
	const void* data;
	void* map_object;
} wb_file_mapping;

void wb_memory_map(const char* path, wb_file_mapping* mapping);
void wb_memory_unmap(wb_file_mapping* mapping);