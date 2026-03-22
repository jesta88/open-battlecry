#pragma once

void* wk_load_library(const char* name);
void wk_unload_library(void* library);
void* wk_load_function(void* library, const char* name);