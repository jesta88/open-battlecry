#pragma once

enum log_type
{
    LOG_INFO,
    LOG_DEBUG,
    LOG_ERROR
};

#ifdef __GNUC__
void log_printf(enum log_type log_type, const char* file_name,
                int line, const char* function, const char* format, ...) __attribute__ ((format (printf, 5, 6)));
#else
void log_printf(enum log_type log_type, const char* file_name,
                int line, const char* function, const char* format, ...);
#endif

#ifdef _MSC_VER
#define log_info(format, ...) log_printf(LOG_INFO, __FILE__, __LINE__, __func__, format, __VA_ARGS__)
#define log_error(format, ...) log_printf(LOG_ERROR, __FILE__, __LINE__, __func__, format, __VA_ARGS__)
#ifndef NDEBUG
#define log_debug(format, ...) log_printf(LOG_DEBUG, __FILE__, __LINE__, __func__, format, __VA_ARGS__)
#else
#define log_debug(format, ...)
#endif
#else
#define log_info(format, args...) log_printf(LOG_INFO, __FILE__, __LINE__, __func__, format, ## args)
#define log_error(format, args...) log_printf(LOG_ERROR, __FILE__, __LINE__, __func__, format, ## args)
#ifndef NDEBUG
#define log_debug(format, args...) log_printf(LOG_DEBUG, __FILE__, __LINE__, __func__, format, ## args)
#else
#define log_debug(format, args...)
#endif
#endif