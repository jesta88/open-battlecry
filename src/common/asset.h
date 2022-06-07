#pragma once

#include "types.h"

typedef struct pak_t pak_t;

void pak_load_from_file(const char* file_name, struct pak_t* dst);
void pak_load_from_memory(u8* data, u32 data_size, struct pak_t* dst);
void pak_unload(struct pak_t* pak);