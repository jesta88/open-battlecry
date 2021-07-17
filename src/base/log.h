#pragma once

#define _LINE_TOK(LINE) #LINE
#define _LINE_TOK2(LINE) _LINE_TOK(LINE)
#define FILE_LINE "[" __FILE__ ":" _LINE_TOK2(__LINE__) "] "

enum log_type
{
    LOG_INFO,
    LOG_DEBUG,
    LOG_ERROR
};

void log_printf(enum log_type log_type, const char* file_name,
                int line, const char* function, const char* format, ...);

#define log_info(...) log_printf(LOG_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_error(...) log_printf(LOG_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#ifdef DEBUG
#define log_debug(...) log_printf(LOG_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define log_debug(...) void(0)
#endif
