#pragma once

#include "../types.h"

typedef struct WbWindow WbWindow;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;

typedef struct WbRenderer
{
    SDL_Renderer* sdl_renderer;
    SDL_Texture* screen_texture;
} WbRenderer;

typedef struct WbTexture
{
    SDL_Texture* sdl_texture;
} WbTexture;

WbRenderer* wbCreateRenderer(WbWindow* window, WbTempAllocator* allocator);
void wbDestroyRenderer(WbRenderer* renderer);

// TODO: Replace temp allocator by a more appropriate allocator
WbTexture* wbCreateTexture(const WbRenderer* renderer, const char* file_name, WbTempAllocator* allocator);

void wbRendererPresent(const WbRenderer* renderer);

void wbRenderSelectionBox(const WbRenderer* renderer, const WbRect* rect);

