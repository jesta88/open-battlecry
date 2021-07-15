#pragma once

#include <stdint.h>

typedef struct WbWindow WbWindow;
typedef struct WbConfig WbConfig;
typedef struct SDL_Window SDL_Window;

extern WbConfig* c_window_width;
extern WbConfig* c_window_height;
extern WbConfig* c_window_fullscreen;
extern WbConfig* c_window_borderless;

WbWindow* wbCreateWindow(const char* title);
void wbDestroyWindow(WbWindow* window);
void wbHandleWindowEvents(void);

void wbWindowGetSize(const WbWindow* window, uint16_t* width, uint16_t* height);
void wbWindowSetTitle(const WbWindow* window, const char* title);

SDL_Window* wbSdlWindow(const WbWindow* window);
