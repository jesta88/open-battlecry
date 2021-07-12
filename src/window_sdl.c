#include "window.h"
#include "cvar.h"
#include "log.h"
#include "bits.h"
#include "assert.h"
#include <SDL2/SDL_video.h>

static WbCvar cvar_window_title;
static WbCvar cvar_window_width;
static WbCvar cvar_window_height;
static WbCvar cvar_window_flags;

void wbInitWindow(WbWindow* window)
{
    WB_ASSERT(window != NULL);

    wbCvarRegister

    int32_t position = SDL_WINDOWPOS_UNDEFINED;

    bool borderless = wbHasFlag8(cvar_window_flags.int_value, WB_WINDOW_BORDERLESS);
    bool fullscreen = wbHasFlag8(cvar_window_flags.int_value, WB_WINDOW_FULLSCREEN);

    uint32_t flags = SDL_WINDOW_ALLOW_HIGHDPI;
    if (fullscreen && borderless) flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN_DESKTOP;
    else if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

    window->sdl_window = SDL_CreateWindow(cvar_window_title.string_value, position, position,
                                          cvar_window_width.int_value, cvar_window_height.int_value, flags);
    if (window->sdl_window == NULL)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }
}

void wbShutdownWindow(WbWindow* window)
{
    WB_ASSERT(window != NULL);

    SDL_DestroyWindow(window->sdl_window);
}
