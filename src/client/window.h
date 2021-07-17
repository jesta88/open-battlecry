#pragma once

#include <stdint.h>

struct window;
struct SDL_Window;

extern struct config* c_window_width;
extern struct config* c_window_height;
extern struct config* c_window_fullscreen;
extern struct config* c_window_borderless;

struct window* c_create_window(const char* title);
void wbDestroyWindow(struct window* window);
void wbHandleWindowEvents(void);

void wbWindowGetSize(const struct window* window, uint16_t* width, uint16_t* height);
void wbWindowSetTitle(const struct window* window, const char* title);

struct SDL_Window* wbSdlWindow(const struct window* window);
