#pragma once

#include <stddef.h>
#include <stdint.h>

// Read an entire file into a malloc'd buffer. Caller must free().
// Returns NULL on failure. Sets *out_size to the number of bytes read.
uint8_t* read_file_binary(const char* path, size_t* out_size);
