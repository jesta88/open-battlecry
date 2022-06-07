#include "filesystem.h"
#include "windows.h"
#include "log.h"
#include <stdio.h>
#include <assert.h>

wb_file wb_file_size(const char* file_name, u32* file_size, bool close)
{
	FILE* file = fopen(file_name, "rb");
    if (!file)
    {
        wb_log_error("Failed to open file: %s", file_name);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    rewind(file);
    if (close)
    {
        fclose(file);
        return NULL;
    }
    else
    {
        return file;
    }
}

void wb_file_map(const char* path, wb_file_mapping* mapping)
{
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
}

void wb_file_unmap(wb_file_mapping* mapping)
{
	assert(mapping != NULL);

	if (mapping->data != NULL)
		UnmapViewOfFile(mapping->data);
	if (mapping->map_object != NULL)
		CloseHandle(mapping->map_object);
}
