#pragma once

typedef enum
{
	WB_LOG_INFO,
	WB_LOG_DEBUG,
	WB_LOG_ERROR
} WbLogType;

#ifdef __GNUC__
void wb_log(WbLogType log_type, const char* file_name,
                int line, const char* function, const char* format, ...) __attribute__ ((format (printf, 5, 6)));
#else
void wb_log(WbLogType log_type, const char* file_name,
                int line, const char* func, const char* format, ...);
#endif

#define wb_log_info(format, ...) wb_log(WB_LOG_INFO, __FILE__, __LINE__, __func__, format "\n", ##__VA_ARGS__)
#define wb_log_error(format, ...) wb_log(WB_LOG_ERROR, __FILE__, __LINE__, __func__, format "\n", ##__VA_ARGS__)
#ifndef NDEBUG
#define wb_log_debug(format, ...) wb_log(WB_LOG_DEBUG, __FILE__, __LINE__, __func__, format "\n", ##__VA_ARGS__)
#else
#define wb_log_debug(format, ...)
#endif