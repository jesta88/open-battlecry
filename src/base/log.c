#include "log.h"
#include "stb_sprintf.h"
#include <stdio.h>

static const char* log_type_string[3] = {
    "[INFO]",
    "[DEBUG]",
    "[ERROR]"
};

void log_printf(enum log_type log_type, const char* file_name,
    int line, const char* function, const char* format, ...)
{
    char buffer[2048];

    va_list args;
    va_start(args, format);
    stbsp_vsnprintf(buffer, sizeof(buffer) - 1, format, args);
    va_end(args);

    const char* type = log_type_string[log_type];
    FILE* file = log_type == LOG_ERROR ? stderr : stdout;
    fprintf(file, "%s %s:%i (%s) %s", type, file_name, line, function, buffer);
    fflush(file);
}