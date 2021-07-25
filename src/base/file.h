#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef void* file_t;

enum file_mode
{
    FILE_MODE_READ_WRITE, 
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND
};

enum seek_type
{
    SEEK_FROMSTART,
    SEEK_FROMHERE,
    SEEK_FROMEND,
};

file_t file_open(const char* name, uint8_t mode);
void file_close(file_t file);
uint32_t file_seek(uint32_t position, uint8_t seek_type);
void file_rewind(file_t file);
file_t file_size(const char* file_name, uint32_t* file_size, bool close);
uint32_t file_read(file_t file, uint8_t bytes[], uint32_t bytes_size);
uint32_t file_write(file_t file, uint8_t* bytes, uint32_t bytes_size);