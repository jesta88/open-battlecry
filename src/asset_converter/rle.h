#pragma once

#include "../common/types.h"

#define WB_MAX_RLE_PALETTE 256

typedef enum
{
    WB_PALETTE_TYPE_NORMAL = 0,
    WB_PALETTE_TYPE_DAYTIME = 0,
    WB_PALETTE_TYPE_DARKER = 1,
    WB_PALETTE_TYPE_LIGHTER = 2,
    WB_PALETTE_TYPE_NIGHTTIME = 3,
    WB_PALETTE_TYPE_SUNSET1 = 4,
    WB_PALETTE_TYPE_SUNSET2 = 5,
    WB_PALETTE_TYPE_SUNSET3 = 6,
    WB_PALETTE_TYPE_SUNSET4 = 7,
    WB_PALETTE_TYPE_COUNT = 8
} wb_palette_type;

typedef struct
{
    s32 size;
    s16 width;
    s16 height;
    s16 palettes[WB_PALETTE_TYPE_COUNT][WB_MAX_RLE_PALETTE];
    s32 pointer_block_size;
    s32 total_pointer_block_size;
    s16 pointer_block_count;
} wb_rle_header;

void wb_convert_rle_bc7(u32 rle_size, const u8* rle_bytes, bool shadowed);