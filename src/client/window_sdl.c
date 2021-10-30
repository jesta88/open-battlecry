#include "window.h"
#include "client.h"
#include "../engine/config.h"
#include "../engine/log.h"
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>

enum
{
    MAX_WINDOWS = 8
};

struct window_t
{
    SDL_Window* handle;
};

static window_t windows[MAX_WINDOWS];

window_t* window_create(const char* title)
{
    int32_t position = SDL_WINDOWPOS_UNDEFINED;

    bool borderless = c_window_borderless->bool_value;
    bool fullscreen = c_window_fullscreen->bool_value;

    uint32_t flags = SDL_WINDOW_ALLOW_HIGHDPI;
    if (fullscreen && borderless) flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN_DESKTOP;
    else if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

    sdl_window = SDL_CreateWindow(
        title, position, position,
        c_window_width->int_value, c_window_height->int_value, flags);

    if (sdl_window == NULL)
    {
        log_error("%s", SDL_GetError());
    }

    return sdl_window;
}

void window_destroy(window_t* window)
{
    if (!window)
        return;
    SDL_DestroyWindow(window->handle);
    window->handle = NULL;
}

void window_handle_events(void)
{
    SDL_PumpEvents();

    SDL_Event events[8];
    const int count = SDL_PeepEvents(events, 8, SDL_GETEVENT, SDL_QUIT, SDL_SYSWMEVENT);

    for (int i = 0; i < count; i++)
    {
        SDL_Event* event = &events[i];
        if (event->type == SDL_QUIT)
        {
            c_quit->bool_value = true;
        }
    }
}

void window_get_size(uint16_t* width, uint16_t* height)
{
    int w, h;
    SDL_GetWindowSize(sdl_window, &w, &h);
    *width = w;
    *height = h;
}

void window_set_title(const char* title)
{
    SDL_SetWindowTitle(sdl_window, title);
}
