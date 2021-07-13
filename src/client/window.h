#pragma once

#include "../types.h"

typedef struct SDL_Window SDL_Window;

typedef struct WbWindow
{
    SDL_Window* sdl_window;
} WbWindow;

WbWindow* wbCreateWindow(WbTempAllocator* allocator);
void wbDestroyWindow(WbWindow* window);
void wbWindowSize(const WbWindow* window, uint16_t* width, uint16_t* height);
