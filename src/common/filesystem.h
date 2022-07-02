#pragma once

#include "types.h"

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

u32 wb_filesystem_read(const char* path, u8* buffer);

const char* wb_filesystem_folder_dialog(const char* title, const char* default_path);
void wb_filesystem_directory_iterate(const char* path, void (*file_proc)(const char*));
