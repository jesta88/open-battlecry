#include "log.h"
#include "system.h"

#include <stdarg.h>
#include <stdio.h>

#define COLOR_NORMAL  "\x1B[0m"
#define COLOR_RED     "\x1B[31m"
#define COLOR_CYAN    "\x1B[36m"

#ifdef _MSC_VER
void __stdcall OutputDebugStringA(char* lpOutputString);
void __stdcall OutputDebugStringW(wchar_t* lpOutputString);
#endif

static const char* log_type_string[3] = {
    "[INFO]",
    "[DEBUG]",
    "[ERROR]"
};

static const char* log_color[3] = {
	COLOR_NORMAL,
	COLOR_CYAN,
	COLOR_RED
};

void sys_log(log_type log_type, const char* file_name,
             int line, const char* function, const char* format, ...)
{
    char buffer[2048];

    FILE* file = log_type == LOG_ERROR ? stderr : stdout;
    const char* type = log_type_string[log_type];
    const char* color = log_color[log_type];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer) - 1, format, args);
    va_end(args);

#ifdef _MSC_VER
    OutputDebugStringA(buffer);
#endif

    fprintf(file, "%s%s %s:%i (%s) %s%s", color, type, file_name, line, function, buffer, log_color[0]);
}
