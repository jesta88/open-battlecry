#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct image_t image_t;
typedef struct { uint32_t index; } sprite_t;

extern struct config* c_render_vsync;
extern struct config* c_render_scale;

void renderer_init(void);
void renderer_quit(void);
void renderer_draw(void);
void renderer_present(void);

sprite_t renderer_add_sprite(image_t* image);
void renderer_remove_sprite(sprite_t sprite);
