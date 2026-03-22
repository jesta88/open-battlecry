#include "log.h"
#include <SDL_log.h>
#include <stdarg.h>

void log_print(log_level level, const char* file_name, int line, const char* function, const char* format, ...)
{
    const SDL_LogPriority level_to_priority[] = {
        SDL_LOG_PRIORITY_DEBUG,
        SDL_LOG_PRIORITY_INFO,
        SDL_LOG_PRIORITY_ERROR};

    (void)file_name;
    (void)line;
    (void)function;

    va_list args;
    va_start(args, format);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, level_to_priority[(int)level], format, args);
    va_end(args);
}