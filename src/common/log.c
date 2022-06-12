#include "log.h"
#include <stdarg.h>
#include <stdio.h>

static const char* log_type_string[3] = {
    "[INFO]",
    "[DEBUG]",
    "[ERROR]"
};

static const char* log_color[3] = {
#ifdef _WIN32
    "\x1b[0m",
    "\x1b[96m",
    "\x1b[91m"
#else
    "\033[0m",
    "\x1B[36m",
    "\x1B[31m"
#endif
};

void wbLog(WbLogType log_type, const char* file_name,
                   int line, const char* function, const char* format, ...)
{
    char buffer[2048];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer) - 1, format, args);
    va_end(args);

    FILE* file = log_type == WB_LOG_ERROR ? stderr : stdout;
    const char* type = log_type_string[log_type];
    const char* color = log_color[log_type];

    fprintf(file, "%s%s %s:%i (%s) %s%s\n", color, type, file_name, line, function, buffer, log_color[0]);
    fflush(file);
}
