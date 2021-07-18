#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct SDL_Surface SDL_Surface;

enum image_load_flags
{
    IMAGE_LOAD_TRANSPARENT = 1 << 0
};

typedef struct image_t
{
    SDL_Surface* sdl_surface;
    uint32_t pixel_format;
    uint32_t width;
    uint32_t height;
    uint32_t size;
} image_t;

void image_init_decoders(void);
void image_load(const char* file_name, uint8_t flags, image_t* image);
void image_free(image_t* image);