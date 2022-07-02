#pragma once

#include "types.h"

const char* wb_path_get_extension(const char* path);
void wb_path_rename_extension(char* path, const char* extension);
void wb_path_strip_path(char* path);
const char* wb_path_get_file_name_with_extension(const char* path);