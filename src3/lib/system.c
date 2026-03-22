#include "system.h"

#include <stdlib.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_log.h>

static char     working_dir[1024];
static char     base_dir[1024];
static uint64_t counter_freq;

int sys_init(void)
{
    char* current_dir = SDL_GetCurrentDirectory();
    if (current_dir == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return 1;
    }
    strcpy(working_dir, current_dir);
    SDL_free(current_dir);

    counter_freq = SDL_GetPerformanceFrequency();

    return 0;
}