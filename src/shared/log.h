#pragma once

enum s_log_type
{
    S_LOG_INFO,
    S_LOG_DEBUG,
    S_LOG_ERROR
};

void s_log(enum s_log_type log_type, const char* format, ...);

#define s_log_info(format, ...) s_log(S_LOG_INFO, format, ##__VA_ARGS__)
#define s_log_error(format, ...) s_log(S_LOG_ERROR, format, ##__VA_ARGS__)
#ifdef _DEBUG
#define s_log_debug(format, ...) s_log(S_LOG_DEBUG, format, ##__VA_ARGS__)
#else
#define s_log_debug(format, ...) void(0)
#endif
