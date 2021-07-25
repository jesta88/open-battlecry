#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct font_t font_t;
typedef struct image_t image_t;

typedef struct { uint16_t index; } texture_t;

extern struct config_t* c_render_vsync;
extern struct config_t* c_render_scale;

void renderer_init(void* window_handle);
void renderer_quit(void);
void renderer_draw(void);
void renderer_present(void);

texture_t renderer_create_texture(image_t* image);
void renderer_destroy_texture(texture_t texture);

void renderer_add_sprite(texture_t );
void renderer_add_text(font_t* font, int16_t x, int16_t y, const char* text);