#pragma once

#include "../base/types.h"

typedef struct window_t window_t;
typedef struct config_t config_t;

extern config_t* c_window_width;
extern config_t* c_window_height;
extern config_t* c_window_fullscreen;
extern config_t* c_window_borderless;

window_t* window_create(const char* title);
void window_destroy(window_t* window);
void window_handle_events(void);

void window_get_size(uint16_t* width, uint16_t* height);
void window_set_title(const char* title);
