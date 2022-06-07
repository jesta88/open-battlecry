#pragma once

#include "types.h"

typedef struct SDL_Surface SDL_Surface;

typedef struct image_t
{
    SDL_Surface* sdl_surface;
    u32 pixel_format;
    u32 width;
    u32 height;
    u32 size;
} image_t;

void image_init_decoders(void);
void image_load(const char* file_name, image_t* image);
void image_free(image_t* image);