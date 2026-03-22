#include "image.h"

#define STBI_ONLY_BMP
#define STBI_NO_STDIO
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// RLE format constants
// ---------------------------------------------------------------------------

enum
{
    RLE_MAX_PALETTE     = 256,
    RLE_PALETTE_COUNT   = 8,
    RLE_PIXEL_SHADOW_START = 250,
    RLE_PIXEL_SHADOW_END   = 254,
    RLE_PIXEL_TRANSPARENT  = 255,
};

// Shadow alpha values (from WBC3 source)
static const uint8_t shadow_alpha[] = {
    255 - 152,  // index 0 (pixel 250): alpha 103
    255 - 216,  // index 1 (pixel 251): alpha 39
    255 - 228,  // index 2 (pixel 252): alpha 27
    255 - 240,  // index 3 (pixel 253): alpha 15
    255 - 248,  // index 4 (pixel 254): alpha 7
};

// ---------------------------------------------------------------------------
// On-disk RLE header (follows 2-byte ID: "RL" or "RS")
// ---------------------------------------------------------------------------

#pragma pack(push, 1)

typedef struct
{
    int32_t size;
    int16_t width;
    int16_t height;
    int16_t palettes[RLE_PALETTE_COUNT][RLE_MAX_PALETTE]; // RGB565
    int32_t pointer_block_size;
    int32_t total_pointer_block_size;
    int16_t pointer_block_count;
} rle_header;

#pragma pack(pop)

// ---------------------------------------------------------------------------
// RGB565 -> RGB888 conversion
// ---------------------------------------------------------------------------

static inline uint8_t rgb565_r(int16_t c) { return (uint8_t)(((c >> 11) & 0x1F) << 3); }
static inline uint8_t rgb565_g(int16_t c) { return (uint8_t)(((c >>  5) & 0x3F) << 2); }
static inline uint8_t rgb565_b(int16_t c) { return (uint8_t)(((c >>  0) & 0x1F) << 3); }

// ---------------------------------------------------------------------------
// RLE decoder
// ---------------------------------------------------------------------------

bool image_decode_rle(const uint8_t* data, uint32_t size, image* out)
{
    if (!data || size < 2 + sizeof(rle_header) || !out)
        return false;

    // First 2 bytes: "RL" (no shadow) or "RS" (with shadow)
    bool shadowed = (data[1] == 'S');
    const rle_header* hdr = (const rle_header*)(data + 2);

    if (hdr->width <= 0 || hdr->height <= 0)
        return false;

    uint32_t w = (uint32_t)hdr->width;
    uint32_t h = (uint32_t)hdr->height;
    uint32_t rgba_size = w * h * 4;

    // RLE data starts after header + pointer blocks
    size_t rle_offset = 2 + sizeof(rle_header) + (size_t)hdr->total_pointer_block_size;
    if (rle_offset >= size)
        return false;

    const uint8_t* rle_data = data + rle_offset;
    int32_t rle_data_size = hdr->size - hdr->total_pointer_block_size;
    if (rle_data_size <= 0)
        return false;

    uint8_t* pixels = calloc(rgba_size, 1);
    if (!pixels) return false;

    uint32_t byte_count = 0;
    const int16_t* palette = hdr->palettes[0]; // daytime palette

    for (int32_t i = 0; i < rle_data_size && byte_count < rgba_size; ++i)
    {
        uint8_t val = rle_data[i];

        if (val == RLE_PIXEL_TRANSPARENT)
        {
            // Next byte is count of transparent pixels (255,255 = full blank line)
            if (i + 1 >= rle_data_size) break;
            uint8_t next = rle_data[++i];
            uint32_t count = (next == RLE_PIXEL_TRANSPARENT) ? w : (uint32_t)next;
            uint32_t bytes = count * 4;
            if (byte_count + bytes > rgba_size) bytes = rgba_size - byte_count;
            memset(pixels + byte_count, 0, bytes);
            byte_count += bytes;
        }
        else if (shadowed && val == RLE_PIXEL_SHADOW_START)
        {
            // Next byte is count of shadow pixels
            if (i + 1 >= rle_data_size) break;
            uint8_t count = rle_data[++i];
            uint8_t alpha = shadow_alpha[0];
            for (uint8_t j = 0; j < count && byte_count + 4 <= rgba_size; ++j)
            {
                pixels[byte_count++] = 0;
                pixels[byte_count++] = 0;
                pixels[byte_count++] = 0;
                pixels[byte_count++] = alpha;
            }
        }
        else if (shadowed && val > RLE_PIXEL_SHADOW_START && val <= RLE_PIXEL_SHADOW_END)
        {
            // Single shadow pixel
            if (byte_count + 4 > rgba_size) break;
            uint8_t alpha = shadow_alpha[val - RLE_PIXEL_SHADOW_START];
            pixels[byte_count++] = 0;
            pixels[byte_count++] = 0;
            pixels[byte_count++] = 0;
            pixels[byte_count++] = alpha;
        }
        else
        {
            // Palette color
            if (byte_count + 4 > rgba_size) break;
            int16_t color = palette[val];
            pixels[byte_count++] = rgb565_r(color);
            pixels[byte_count++] = rgb565_g(color);
            pixels[byte_count++] = rgb565_b(color);
            pixels[byte_count++] = 255;
        }
    }

    out->pixels = pixels;
    out->width = w;
    out->height = h;
    return true;
}

// ---------------------------------------------------------------------------
// BMP decoder (via stb_image)
// ---------------------------------------------------------------------------

bool image_decode_bmp(const uint8_t* data, uint32_t size, image* out)
{
    if (!data || size == 0 || !out)
        return false;

    int w, h, channels;
    uint8_t* pixels = stbi_load_from_memory(data, (int)size, &w, &h, &channels, 4); // force RGBA
    if (!pixels)
    {
        fprintf(stderr, "[image] BMP decode failed: %s\n", stbi_failure_reason());
        return false;
    }

    out->pixels = pixels;
    out->width = (uint32_t)w;
    out->height = (uint32_t)h;
    return true;
}

void image_free(image* img)
{
    if (img && img->pixels)
    {
        free(img->pixels);
        img->pixels = NULL;
    }
}
