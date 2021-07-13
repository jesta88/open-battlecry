#include "renderer.h"
#include "window.h"
#include "../allocator.h"
#include "../log.h"
#include "../assert.h"
#include "../config.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_image.h>

static WbConfig cvar_renderer_vsync;

WbRenderer* wbCreateRenderer(WbWindow* window, WbTempAllocator* allocator)
{
    WB_ASSERT(window != NULL);
    WB_ASSERT(allocator != NULL);

    WbRenderer* renderer = wbTempAlloc(allocator, sizeof(WbRenderer));
    if (renderer == NULL)
    {
        WB_LOG_ERROR("Failed to allocate memory for WbRenderer.");
        return NULL;
    }

    wbCvarRegisterBool(&cvar_renderer_vsync, "renderer_vsync", false, true);

    uint32_t flags = SDL_RENDERER_ACCELERATED;
    if (cvar_renderer_vsync.bool_value) flags |= SDL_RENDERER_PRESENTVSYNC;

    renderer->sdl_renderer = SDL_CreateRenderer(window->sdl_window, -1, flags);
    if (renderer->sdl_renderer == NULL)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }

    if (IMG_Init(IMG_INIT_PNG) < 0)
    {
        WB_LOG_ERROR("%s", IMG_GetError());
    }

    uint16_t screen_width, screen_height;
    wbWindowSize(window, &screen_width, &screen_height);

    renderer->screen_texture = SDL_CreateTexture(renderer->sdl_renderer, SDL_PIXELFORMAT_ABGR32,
                                                 SDL_TEXTUREACCESS_TARGET, screen_width, screen_height);
    if (renderer->screen_texture == NULL)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }

    return renderer;
}

void wbDestroyRenderer(WbRenderer* renderer)
{
    WB_ASSERT(renderer != NULL);

    SDL_DestroyTexture(renderer->screen_texture);
    SDL_DestroyRenderer(renderer->sdl_renderer);
}

WbTexture* wbCreateTexture(const WbRenderer* renderer, const char* file_name, WbTempAllocator* allocator)
{
    WB_ASSERT(renderer != NULL);
    WB_ASSERT(allocator != NULL);

    WbTexture* texture = wbTempAlloc(allocator, sizeof(WbTexture));

    SDL_Surface* surface = IMG_Load(file_name);
    if (surface == NULL)
    {
        WB_LOG_ERROR("%s", IMG_GetError());
    }

    SDL_Texture* sdl_texture = SDL_CreateTextureFromSurface(renderer->sdl_renderer, surface);
    if (sdl_texture == NULL)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }
    SDL_FreeSurface(surface);


}

void wbRendererPresent(const WbRenderer* renderer)
{
    WB_ASSERT(renderer != NULL);

    SDL_RenderPresent(renderer->sdl_renderer);
}
