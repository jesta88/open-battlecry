#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct SDL_Surface SDL_Surface;

typedef struct image_t
{
    SDL_Surface* sdl_surface;
    uint32_t pixel_format;
    uint32_t width;
    uint32_t height;
    uint32_t size;
} image_t;

void image_init_decoders(void);
void image_load(const char* file_name, image_t* image);
void image_free(image_t* image);