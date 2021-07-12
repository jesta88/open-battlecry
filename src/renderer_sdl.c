#include "renderer.h"

#include <SDL2/SDL_render.h>

struct WbRenderer
{
    SDL_Renderer* sdl_renderer;
};

static WbRenderer s_renderer;

void wbInitRenderer(WbRenderer** renderer)
{
    s_renderer.sdl_renderer = SDL_CreateRenderer(window, -1, flags);
    if (renderer == NULL)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }
    *renderer = &s_renderer;
}

void wbShutdownRenderer(WbRenderer* renderer)
{

}

void wbRendererPresent(WbRenderer* renderer)
{
    SDL_RenderPresent()
}
