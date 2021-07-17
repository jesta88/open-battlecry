#include "window.h"
#include "client.h"
#include "../base/config.h"
#include "../base/log.h"
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <assert.h>

enum
{
#ifdef DEVELOPMENT
    C_MAX_WINDOWS = 8,
#else
    C_MAX_WINDOWS = 1,
#endif
};

struct window
{
    SDL_Window* sdl_window;
};

static struct window windows[C_MAX_WINDOWS];

struct window* c_create_window(const char* title)
{
    uint8_t window_index = UINT8_MAX;
    for (int i = 0; i < C_MAX_WINDOWS; i++)
    {
        if (windows[i].sdl_window == NULL)
        {
            window_index = i;
            break;
        }
    }
    assert(window_index != UINT8_MAX);

    struct window* window = &windows[window_index];

    int32_t position = SDL_WINDOWPOS_UNDEFINED;

    bool borderless = c_window_borderless->bool_value;
    bool fullscreen = c_window_fullscreen->bool_value;

    uint32_t flags = SDL_WINDOW_ALLOW_HIGHDPI;
    if (fullscreen && borderless) flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN_DESKTOP;
    else if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

    window->sdl_window = SDL_CreateWindow(
        title, position, position,
        c_window_width->int_value, c_window_height->int_value, flags);

    if (window->sdl_window == NULL)
    {
        log_error("%s", SDL_GetError());
    }

    return window;
}

void wbDestroyWindow(struct window* window)
{
    assert(window);

    SDL_DestroyWindow(window->sdl_window);
    window->sdl_window = NULL;
}

void wbHandleWindowEvents(void)
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

void wbWindowGetSize(const struct window* window, uint16_t* width, uint16_t* height)
{
    assert(window);
    assert(window->sdl_window);

    int w, h;
    SDL_GetWindowSize(window->sdl_window, &w, &h);
    *width = w;
    *height = h;
}

void wbWindowSetTitle(const struct window* window, const char* title)
{
    assert(window);

    SDL_SetWindowTitle(window->sdl_window, title);
}

struct SDL_Window* wbSdlWindow(const struct window* window)
{
    assert(window);

    return window->sdl_window;
}
