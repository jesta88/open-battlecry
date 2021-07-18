#include "io.h"
#include "log.h"
#include <stdio.h>
#include <assert.h>

file_t io_file_size(const char* file_name, uint32_t* file_size, bool close)
{
    FILE* file = fopen(file_name, "rb");
    if (!file)
    {
        log_error("Failed to open file: %s", file_name);
        return;
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

void io_read_file(file_t file, uint32_t* data_size, uint8_t data[])
{
    uint32_t file_size = *data_size;
    *data_size = fread(data, 1, file_size, file);
    fclose(file);

    if (*data_size != file_size)
    {
        log_info("Bytes read and file size differ: %i, %i", *data_size, file_size);
    }
}
