#include "window.h"
#include "../allocator.h"
#include "../config.h"
#include "../log.h"
#include "../bits.h"
#include "../assert.h"
#include <SDL2/SDL_video.h>

static WbConfig cvar_window_title;
static WbConfig cvar_window_width;
static WbConfig cvar_window_height;
static WbConfig cvar_window_fullscreen;
static WbConfig cvar_window_borderless;

WbWindow* wbCreateWindow(WbTempAllocator* allocator)
{
    WB_ASSERT(allocator != NULL);

    WbWindow* window = wbTempAlloc(allocator, sizeof(WbWindow));
    if (window == NULL)
    {
        WB_LOG_ERROR("Failed to allocate memory for WbWindow.");
        return NULL;
    }

    wbCvarRegisterString(&cvar_window_title, "window_title", "Battlecry", false);
    wbCvarRegisterInt(&cvar_window_width, "window_width", 1280, true);
    wbCvarRegisterInt(&cvar_window_height, "window_height", 720, true);
    wbCvarRegisterBool(&cvar_window_fullscreen, "window_fullscreen", false, true);
    wbCvarRegisterBool(&cvar_window_borderless, "window_borderless", false, true);

    int32_t position = SDL_WINDOWPOS_UNDEFINED;

    bool borderless = cvar_window_borderless.bool_value;
    bool fullscreen = cvar_window_fullscreen.bool_value;

    uint32_t flags = SDL_WINDOW_ALLOW_HIGHDPI;
    if (fullscreen && borderless) flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN_DESKTOP;
    else if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

    window->sdl_window = SDL_CreateWindow(cvar_window_title.string_value, position, position,
                                          cvar_window_width.int_value, cvar_window_height.int_value, flags);
    if (window->sdl_window == NULL)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }

    return window;
}

void wbDestroyWindow(WbWindow* window)
{
    WB_ASSERT(window != NULL);

    SDL_DestroyWindow(window->sdl_window);
}

void wbWindowSize(const WbWindow* window, uint16_t* width, uint16_t* height)
{
    WB_ASSERT(window != NULL);
    WB_ASSERT(window->sdl_window != NULL);

    int w, h;
    SDL_GetWindowSize(window->sdl_window, &w, &h);
    *width = w;
    *height = h;
}
