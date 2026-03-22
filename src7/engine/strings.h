#pragma once

#include "std.h"

ENGINE_API void str_remove_path(char* path);
ENGINE_API void str_remove_file_name(char* path);
ENGINE_API void str_remove_extension(char* path);
ENGINE_API void str_rename_extension(char* path, const char* extension);
ENGINE_API const char* str_find_extension(const char* path);