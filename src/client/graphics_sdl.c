#include "graphics.h"
#include "../base/log.h"
#include "../base/config.h"
#include <SDL2/SDL_render.h>

enum
{
    MAX_DRIVERS = 4
};

static SDL_RendererInfo driver_infos[MAX_DRIVERS];
static uint32_t driver_count;

static SDL_Renderer* renderer;

static void find_render_drivers(void);

void graphics_init(void* window_handle)
{
    find_render_drivers();

    if (!window_handle)
    {
        log_error("%s", "Window handle is null.");
        return;
    }

    uint32_t flags = SDL_RENDERER_ACCELERATED;
    if (c_vsync->bool_value) flags |= SDL_RENDERER_PRESENTVSYNC;

    renderer = SDL_CreateRenderer((SDL_Window*)window_handle, -1, flags);
    if (!renderer)
    {
        log_error("%s", SDL_GetError());
        return;
    }
}

void graphics_quit(void)
{
    SDL_DestroyRenderer(renderer);
}

static void find_render_drivers(void)
{
    driver_count = SDL_GetNumRenderDrivers();
    if (driver_count > MAX_DRIVERS)
    {
        log_error("Found %i render drivers, max supported is %i.", driver_count, MAX_DRIVERS);
    }
    for (int i = 0; i < driver_count; i++)
    {
        SDL_GetRenderDriverInfo(i, &driver_infos[i]);
        log_info("Found render driver: %s", driver_infos[i].name);
    }
}