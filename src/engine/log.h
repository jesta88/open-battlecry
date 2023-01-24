#pragma once

#include "std.h"

typedef enum
{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_ERROR = 2
} log_level;

ENGINE_API void log_print(log_level level, const char* file_name, int line, const char* function, const char* format, ...);

#define log_info(format, ...) log_print(LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, "" format "\n", ##__VA_ARGS__)
#define log_error(format, ...) log_print(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, "" format "\n", ##__VA_ARGS__)
#ifndef NDEBUG
#define log_debug(format, ...) log_print(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, "" format "\n", ##__VA_ARGS__)
#else
#define log_debug(format, ...)
#endif