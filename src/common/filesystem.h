#pragma once

#include "types.h"

typedef void* wb_file;

typedef enum 
{
	FILE_MODE_READ_WRITE,
	FILE_MODE_READ,
	FILE_MODE_WRITE,
	FILE_MODE_APPEND
} wb_file_mode;

typedef enum 
{
	SEEK_FROMSTART,
	SEEK_FROMHERE,
	SEEK_FROMEND,
} wb_seek_mode;

typedef struct
{
	const void* data;
	void* map_object;
} wb_file_mapping;

wb_file wb_file_open(const char* name, u8 mode);
void wb_file_close(wb_file file);
u32 wb_file_seek(u32 position, u8 seek_type);
void wb_file_rewind(wb_file file);
wb_file wb_file_size(const char* file_name, u32* file_size, bool close);
u32 wb_file_read(wb_file file, u8 bytes[], u32 bytes_size);
u32 wb_file_write(wb_file file, u8* bytes, u32 bytes_size);

void wb_file_map(const char* path, wb_file_mapping* mapping);
void wb_file_unmap(wb_file_mapping* mapping);