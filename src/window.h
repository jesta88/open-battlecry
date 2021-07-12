#pragma once

#include "types.h"

typedef struct SDL_Window SDL_Window;

typedef enum WbWindowFlags
{
    WB_WINDOW_FULLSCREEN = 1 << 0,
    WB_WINDOW_BORDERLESS = 1 << 1
} WbWindowFlags;

typedef struct WbWindow
{
    SDL_Window* sdl_window;
} WbWindow;

void wbInitWindow(WbWindow* window);
void wbShutdownWindow(WbWindow* window);

