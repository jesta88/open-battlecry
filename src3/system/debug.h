#pragma once

#include "defines.h"

#if WK_DEBUG
#   define WK_FATAL_PREFIX 
#   define WK_FATAL_SUFFIX
#elif defined(_MSC_VER)
#   define WK_FATAL_PREFIX __declspec(noreturn)
#   define WK_FATAL_SUFFIX
#else
#   define WK_FATAL_PREFIX 
#   define WK_FATAL_SUFFIX __attribute__((noreturn))
#endif

typedef enum
{
    WK_LOG_LEVEL_DEBUG,
    WK_LOG_LEVEL_INFO,
    WK_LOG_LEVEL_ERROR,
    WK_LOG_LEVEL_FATAL,
    WK_LOG_LEVEL_COUNT
} wk_log_level;

void wk_output_debug(const char* message, ...);

void wk_log(wk_log_level level, const char* file_name, int line, const char* message, ...) wk_printf(4, 5);

#if WK_DEBUG
#define wk_log_info(message, ...)  wk_log(WK_LOG_LEVEL_INFO, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define wk_log_error(message, ...) wk_log(WK_LOG_LEVEL_ERROR, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define wk_log_debug(message, ...) wk_log(WK_LOG_LEVEL_DEBUG, __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
#define wk_log_info(...)  (void)0
#define wk_log_error(...) (void)0
#define wk_log_debug(...) (void)0
#endif

WK_FATAL_PREFIX void wk_fatal(const char* message, ...) WK_FATAL_SUFFIX wk_printf(1, 2);