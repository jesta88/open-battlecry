#include "log.h"
#include <stdarg.h>
#include <stdio.h>

#define COLOR_NORMAL  "\x1B[0m"
#define COLOR_RED     "\x1B[31m"
#define COLOR_GREEN   "\x1B[32m"
#define COLOR_YELLOW  "\x1B[33m"
#define COLOR_BLUE    "\x1B[34m"
#define COLOR_MAGENTA "\x1B[35m"
#define COLOR_CYAN    "\x1B[36m"
#define COLOR_WHITE   "\x1B[37m"

void __stdcall OutputDebugStringA(char* lpOutputString);
void __stdcall OutputDebugStringW(wchar_t* lpOutputString);

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

void wb_log(WbLogType log_type, const char* file_name,
                   int line, const char* function, const char* format, ...)
{
    char buffer[2048];

    FILE* file = log_type == WB_LOG_ERROR ? stderr : stdout;
    const char* type = log_type_string[log_type];
    const char* color = log_color[log_type];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer) - 1, format, args);
    va_end(args);

#ifdef _MSC_VER
    OutputDebugStringA(buffer);
#endif

    fprintf(file, "%s%s %s:%i (%s) %s%s\n", color, type, file_name, line, function, buffer, log_color[0]);
    //fflush(file);
}
