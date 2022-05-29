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

wb_file wb_file_open(const char* name, uint8_t mode);
void wb_file_close(wb_file file);
uint32_t wb_file_seek(uint32_t position, uint8_t seek_type);
void wb_file_rewind(wb_file file);
wb_file wb_file_size(const char* file_name, uint32_t* file_size, bool close);
uint32_t wb_file_read(wb_file file, uint8_t bytes[], uint32_t bytes_size);
uint32_t wb_file_write(wb_file file, uint8_t* bytes, uint32_t bytes_size);

void wb_file_map(const char* path, wb_file_mapping* mapping);
void wb_file_unmap(wb_file_mapping* mapping);