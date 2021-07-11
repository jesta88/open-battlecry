#include "log.h"
#include <SDL2/SDL_log.h>

static int log_type_to_sdl[3] = {
    SDL_LOG_PRIORITY_INFO,
    SDL_LOG_PRIORITY_DEBUG,
    SDL_LOG_PRIORITY_ERROR
};

void wbLog(WbLogType log_type, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, log_type_to_sdl[(int)log_type], format, args);
    va_end(args);
}
