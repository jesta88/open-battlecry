#pragma once

#include <stdint.h>
#include <stdbool.h>

enum image_load_flags
{
    IMAGE_LOAD_TRANSPARENT = 1 << 0
};

typedef struct
{
    uint32_t pixel_format;
    uint32_t width;
    uint32_t height;
    uint32_t size;
} image_t;

void image_init_decoders(void);
void image_load_png(const char* file_name, bool transparent, uint8_t flags, struct image* image);