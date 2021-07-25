#include "io.h"
#include "log.h"
#include "file.h"
#include <Windows.h>
#include <assert.h>

file_t file_open(const char* name, uint8_t mode)
{
    return NULL;
}

void file_close(file_t file)
{}

uint32_t file_seek(uint32_t position, uint8_t seek_type)
{
    return 0;
}

void file_rewind(file_t file)
{}

uint32_t file_write(file_t file, uint8_t* bytes, uint32_t bytes_size)
{
    return 0;
}

file_t file_size(const char* file_name, uint32_t* file_size, bool close)
{
    /*FILE* file = fopen(file_name, "rb");
    if (!file)
    {
        log_error("Failed to open file: %s", file_name);
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
    }*/

    return NULL;
}

uint32_t file_read(file_t file, uint8_t bytes[], uint32_t bytes_size)
{
    /*uint32_t bytes_read = fread(bytes, 1, file_size, file);
    fclose(file);

    if (bytes_read != bytes_size)
    {
        log_info("Bytes read and bytes size differ: %i, %i", bytes_read, bytes_size);
    }*/

    return 0;
}
