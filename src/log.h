#pragma once

typedef enum WbLogType{
    WB_LOG_INFO,
    WB_LOG_DEBUG,
    WB_LOG_ERROR
} WbLogType;

void wbLog(WbLogType log_type, const char* format, ...);

#define WB_LOG_INFO(format, ...) wbLog(WB_LOG_INFO, format, ##__VA_ARGS__)
#define WB_LOG_DEBUG(format, ...) wbLog(WB_LOG_DEBUG, format, ##__VA_ARGS__)
#define WB_LOG_ERROR(format, ...) wbLog(WB_LOG_ERROR, format, ##__VA_ARGS__)