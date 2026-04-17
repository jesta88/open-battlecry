#include "image.h"
#include "baked_format.h"

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
// Raw RGBA texture decoder (.tex format)
// ---------------------------------------------------------------------------

bool image_decode_tex(const uint8_t* data, uint32_t size, image* out)
{
    if (!data || size < sizeof(baked_tex_header) || !out)
        return false;

    const baked_tex_header* hdr = (const baked_tex_header*)data;
    if (hdr->magic != BAKED_TEX_MAGIC)
        return false;

    uint32_t pixel_size = hdr->width * hdr->height * 4;
    if (size < sizeof(baked_tex_header) + pixel_size)
        return false;

    uint8_t* pixels = malloc(pixel_size);
    if (!pixels) return false;

    memcpy(pixels, data + sizeof(baked_tex_header), pixel_size);

    out->pixels = pixels;
    out->width = hdr->width;
    out->height = hdr->height;
    return true;
}

// ---------------------------------------------------------------------------
// Fast BMP decoder (hand-written, replaces stb_image for BMP)
// Handles 24-bit and 32-bit uncompressed BMPs (bottom-up and top-down).
// ---------------------------------------------------------------------------

bool image_decode_bmp(const uint8_t* data, uint32_t size, image* out)
{
    if (!data || size < 54 || !out)
        return false;

    // BMP magic
    if (data[0] != 'B' || data[1] != 'M')
        return false;

    // BITMAPFILEHEADER (14 bytes)
    uint32_t pixel_offset = *(const uint32_t*)(data + 10);

    // BITMAPINFOHEADER (40 bytes, starts at offset 14)
    int32_t width  = *(const int32_t*)(data + 18);
    int32_t height = *(const int32_t*)(data + 22);
    uint16_t bpp   = *(const uint16_t*)(data + 28);
    uint32_t compression = *(const uint32_t*)(data + 30);

    if (width <= 0 || height == 0)
        return false;

    // Only uncompressed (BI_RGB=0, BI_BITFIELDS=3 for 32-bit)
    if (compression != 0 && compression != 3)
        return false;
    if (bpp != 8 && bpp != 24 && bpp != 32)
        return false;

    bool bottom_up = (height > 0);
    uint32_t abs_w = (uint32_t)width;
    uint32_t abs_h = bottom_up ? (uint32_t)height : (uint32_t)(-height);

    // Row stride (rows are padded to 4-byte alignment)
    uint32_t src_stride;
    if (bpp == 8)       src_stride = (abs_w + 3) & ~3u;
    else if (bpp == 24) src_stride = (abs_w * 3 + 3) & ~3u;
    else                src_stride = abs_w * 4;

    if (pixel_offset + (uint64_t)src_stride * abs_h > size)
        return false;

    // For 8-bit: read the palette (located right after the 40-byte BITMAPINFOHEADER at offset 54)
    uint8_t palette[256][4]; // BGRA
    if (bpp == 8)
    {
        uint32_t num_colors = *(const uint32_t*)(data + 46); // biClrUsed
        if (num_colors == 0) num_colors = 256;
        if (num_colors > 256) num_colors = 256;
        uint32_t palette_offset = 14 + *(const uint32_t*)(data + 14); // 14 + biSize
        if (palette_offset + num_colors * 4 > size) return false;
        memcpy(palette, data + palette_offset, num_colors * 4);
    }

    uint8_t* pixels = malloc(abs_w * abs_h * 4);
    if (!pixels) return false;

    const uint8_t* src = data + pixel_offset;

    for (uint32_t y = 0; y < abs_h; y++)
    {
        uint32_t src_y = bottom_up ? (abs_h - 1 - y) : y;
        const uint8_t* row = src + src_y * src_stride;
        uint8_t* dst = pixels + y * abs_w * 4;

        if (bpp == 32)
        {
            for (uint32_t x = 0; x < abs_w; x++)
            {
                dst[x * 4 + 0] = row[x * 4 + 2]; // R from B
                dst[x * 4 + 1] = row[x * 4 + 1]; // G
                dst[x * 4 + 2] = row[x * 4 + 0]; // B from R
                dst[x * 4 + 3] = row[x * 4 + 3]; // A
            }
        }
        else if (bpp == 24)
        {
            for (uint32_t x = 0; x < abs_w; x++)
            {
                dst[x * 4 + 0] = row[x * 3 + 2]; // R
                dst[x * 4 + 1] = row[x * 3 + 1]; // G
                dst[x * 4 + 2] = row[x * 3 + 0]; // B
                dst[x * 4 + 3] = 255;             // A (opaque)
            }
        }
        else // 8-bit indexed
        {
            for (uint32_t x = 0; x < abs_w; x++)
            {
                uint8_t idx = row[x];
                dst[x * 4 + 0] = palette[idx][2]; // R from B
                dst[x * 4 + 1] = palette[idx][1]; // G
                dst[x * 4 + 2] = palette[idx][0]; // B from R
                dst[x * 4 + 3] = 255;             // A (opaque)
            }
        }
    }

    out->pixels = pixels;
    out->width = abs_w;
    out->height = abs_h;
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
