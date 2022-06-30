#include "rle.h"
#include "../common/log.h"
#include <stdio.h>

enum
{
    PIXEL_SHADOW_START = 250,
    PIXEL_SHADOW_END = 254,
    PIXEL_SHADOW_COUNT = PIXEL_SHADOW_END - PIXEL_SHADOW_START + 1,
    PIXEL_TRANSPARENT = 255,

    PIXEL_SIDE_START = 224,
    PIXEL_SIZE_END = 238
};

static const u8 shadow_alpha[PIXEL_SHADOW_COUNT] = {
    255 - 152, 255 - 216, 255 - 228, 255 - 240, 255 - 248
};

static inline u8 convert_565_to_R(u16 pixel) { return (((u8) ((pixel >> 11) & 0x001F)) << 3); }
static inline u8 convert_565_to_G(u16 pixel) { return (((u8) ((pixel >> 5) & 0x003F)) << 2); }
static inline u8 convert_565_to_B(u16 pixel) { return (((u8) ((pixel >> 0) & 0x001F)) << 3); }

void wb_convert_rle_bc7(u32 rle_size, const u8* rle_data, bool shadowed)
{
    if (rle_size == 0 || rle_data == NULL)
    {
        wb_log_error("Invalid RLE data");
        return;
    }

    wb_rle_header rle_header = {0};

    // RLE Encoding
    s32 size = rle_header.size - rle_header.total_pointer_block_size;

    u64 row_size = rle_header.width * sizeof(u8) * 4;
    for (int y = 0; y < rle_header.height; y++)
    {
        //png_rows[y] = png_malloc(png_ptr, row_size);
    }

    int byte_count = 0;
    int row = 0;
    for (int pixel = 0; pixel < size; pixel++)
    {
        s16 value = rle_data[pixel];

        if (value == 255)
        {
            s16 count = rle_data[pixel + 1];

            // Two consecutive 255 means a blank line
            if (pixel < size - 1 && rle_data[pixel + 1] == 255)
            {
                count = rle_header.width;
            }

            for (s16 i = 0; i < count; i++)
            {
                if (byte_count >= row_size)
                {
                    row++;
                    //png_row = png_rows[row];
                    byte_count = 0;
                }

//                 png_row[byte_count++] = 0;
//                 png_row[byte_count++] = 0;
//                 png_row[byte_count++] = 0;
//                 png_row[byte_count++] = 0;
            }
            pixel++;
        }
        else if (shadowed && value == PIXEL_SHADOW_START)
        {
            u8 alpha = shadow_alpha[value - PIXEL_SHADOW_START];
            s16 count = rle_data[pixel + 1];

            for (s16 i = 0; i < count; i++)
            {
                if (byte_count >= row_size)
                {
                    row++;
                    //png_row = png_rows[row];
                    byte_count = 0;
                }

//                 png_row[byte_count++] = shadow_color;
//                 png_row[byte_count++] = shadow_color;
//                 png_row[byte_count++] = shadow_color;
//                 png_row[byte_count++] = alpha;
            }
            pixel++;
        }
        else if (shadowed && value > PIXEL_SHADOW_START)
        {
            u8 alpha = shadow_alpha[value - PIXEL_SHADOW_START];

            if (byte_count >= row_size)
            {
                row++;
                //png_row = png_rows[row];
                byte_count = 0;
            }

//             png_row[byte_count++] = shadow_color;
//             png_row[byte_count++] = shadow_color;
//             png_row[byte_count++] = shadow_color;
//             png_row[byte_count++] = alpha;
        }
        else
        {
            s16 color = rle_header.palettes[0][value];

            if (byte_count >= row_size)
            {
                row++;
                //png_row = png_rows[row];
                byte_count = 0;
            }

//             png_row[byte_count++] = convert_565_to_R(color);
//             png_row[byte_count++] = convert_565_to_G(color);
//             png_row[byte_count++] = convert_565_to_B(color);
//             png_row[byte_count++] = 255;
        }
    }
}
