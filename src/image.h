#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint8_t* pixels;   // RGBA32, caller must call image_free()
    uint32_t width;
    uint32_t height;
} image;

// Decode an RLE sprite from raw XCR resource data. Returns false on failure.
bool image_decode_rle(const uint8_t* data, uint32_t size, image* out);

// Decode a BMP image from raw XCR resource data. Returns false on failure.
bool image_decode_bmp(const uint8_t* data, uint32_t size, image* out);

// Free pixel data.
void image_free(image* img);
