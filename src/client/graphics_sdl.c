#include "graphics.h"
#include "../base/log.h"
#include <SDL2/SDL_render.h>

enum
{
    MAX_DRIVERS = 4
};

static SDL_RendererInfo driver_infos[MAX_DRIVERS];
static uint32_t driver_count;

static SDL_Renderer* renderer;

static void find_render_drivers(void);

void graphics_init(void)
{
    find_render_drivers();

    renderer = 
}

void graphics_quit(void)
{
    
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