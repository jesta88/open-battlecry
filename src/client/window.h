#pragma once

#include <stdint.h>

typedef struct SDL_Window SDL_Window;

extern struct config* c_window_width;
extern struct config* c_window_height;
extern struct config* c_window_fullscreen;
extern struct config* c_window_borderless;

void window_init(const char* title);
void window_quit(void);
void window_handle_events(void);

void window_get_size(uint16_t* width, uint16_t* height);
void window_set_title(const char* title);

SDL_Window* window_get_sdl_window(void);
