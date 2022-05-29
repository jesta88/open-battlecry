#pragma once

#include <core/types.h>

#define MAX_RLE_PALETTE 256

typedef enum
{
    PALETTE_TYPE_NORMAL = 0,
    PALETTE_TYPE_DAYTIME = 0,
    PALETTE_TYPE_DARKER = 1,
    PALETTE_TYPE_LIGHTER = 2,
    PALETTE_TYPE_NIGHTTIME = 3,
    PALETTE_TYPE_SUNSET1 = 4,
    PALETTE_TYPE_SUNSET2 = 5,
    PALETTE_TYPE_SUNSET3 = 6,
    PALETTE_TYPE_SUNSET4 = 7,
    PALETTE_TYPE_COUNT = 8
} palette_type_t;

typedef struct
{
    int32_t size;
    int16_t width;
    int16_t height;
    int16_t palettes[PALETTE_TYPE_COUNT][MAX_RLE_PALETTE];
    int32_t pointer_block_size;
    int32_t total_pointer_block_size;
    int16_t pointer_block_count;
} rle_header_t;