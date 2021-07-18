#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef void* file_t;

file_t io_file_size(const char* file_name, uint32_t* file_size, bool close);
void io_read_file(file_t file, uint32_t* data_size, uint8_t data[]);