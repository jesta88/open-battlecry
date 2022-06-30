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