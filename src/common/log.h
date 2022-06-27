#pragma once

typedef enum
{
	WB_LOG_INFO,
	WB_LOG_DEBUG,
	WB_LOG_ERROR
} WbLogType;

#ifdef __GNUC__
void wbLog(WbLogType log_type, const char* file_name,
                int line, const char* function, const char* format, ...) __attribute__ ((format (printf, 5, 6)));
#else
void wbLog(WbLogType log_type, const char* file_name,
                int line, const char* func, const char* format, ...);
#endif

#ifdef _MSC_VER
#define wbLogInfo(format, ...) wbLog(WB_LOG_INFO, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define wbLogError(format, ...) wbLog(WB_LOG_ERROR, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#ifndef NDEBUG
#define wbLogDebug(format, ...) wbLog(WB_LOG_DEBUG, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#else
#define wbLogDebug(format, ...)
#endif
#else
#define wbLogInfo(format, args...) wbLog(WB_LOG_INFO, __FILE__, __LINE__, __func__, format, ##args)
#define wbLogError(format, args...) wbLog(WB_LOG_ERROR, __FILE__, __LINE__, __func__, format, ##args)
#ifndef NDEBUG
#define wbLogDebug(format, args...) wbLog(WB_LOG_DEBUG, __FILE__, __LINE__, __func__, format, ##args)
#else
#define wbLogDebug(format, args...)
#endif
#endif