#include "font.h"
#include "image.h"
#include "log.h"
#include "bits.inl"
#include "string.inl"
#include <assert.h>
#include <stdio.h>

enum
{
    MAX_FONT_NAME_LENGTH = 64,
};

typedef struct fnt_info_t
{
    s16 font_size;
    wb_bitset8 bit_field;
    u8 char_set;
    u16 stretch_height;
    u8 anti_aliasing;
    u8 padding_up;
    u8 padding_right;
    u8 padding_down;
    u8 padding_left;
    u8 spacing_horizontal;
    u8 spacing_vertical;
    u8 outline;
    char font_name[MAX_FONT_NAME_LENGTH];
} fnt_info_t;

typedef struct fnt_common_t
{
    u16 line_height;
    u16 base;
    u16 scale_width;
    u16 scale_height;
    u16 pages;
    wb_bitset8 bit_field;
    u8 alpha_channel;
    u8 red_channel;
    u8 green_channel;
    u8 blue_channel;
} fnt_common_t;

typedef struct fnt_pages_t
{
    char page_names[4][MAX_FONT_NAME_LENGTH];
} fnt_pages_t;

typedef struct fnt_char_t
{
    u32 id;
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    s16 x_offset;
    s16 y_offset;
    s16 advance;
    u8 page;
    u8 channel;
} fnt_char_t;

typedef struct fnt_kerning_t
{
    u32 first;
    u32 second;
    s16 amount;
} fnt_kerning_t;


void font_load(const char* file_name, font_t* font)
{
    FILE* fnt_file = fopen(file_name, "rb");
    if (!fnt_file)
    {
        wb_log_error("Failed to open file: %s", file_name);
        return;
    }

    {
        char identifier[4];
        fread(identifier, 1, sizeof(identifier), fnt_file);
        if (identifier[0] != 66 || identifier[1] != 77 || identifier[2] != 70)
        {
            wb_log_error("File is not a bmfont file: %s", file_name);
            return;
        }
    }
    {
        u8 block_type;
        while (fread(&block_type, sizeof(block_type), 1, fnt_file))
        {
            u32 block_size;
            fread(&block_size, sizeof(block_size), 1, fnt_file);
            if (block_type < 4)
            {
                fseek(fnt_file, (int) block_size, SEEK_CUR);
            }
            else
            {
                const u32 char_count = block_size / (u32) sizeof(fnt_char_t);
                assert(char_count <= MAX_FONT_GLYPHS);

                font->glyph_count = char_count;

                fnt_char_t chars[MAX_FONT_GLYPHS];
                fread(chars, block_size, 1, fnt_file);

                for (u32 i = 0; i < char_count; i++)
                {
                    fnt_char_t* c = &chars[i];
                    glyph_t* glyph = &font->glyphs[c->id];
                    glyph->x = c->x;
                    glyph->y = c->y;
                    glyph->width = c->width;
                    glyph->height = c->height;
                    glyph->x_offset = c->x_offset;
                    glyph->y_offset = c->y_offset;
                    glyph->advance = c->advance;
                }
            }
        }
    }
    fclose(fnt_file);

    char png_path[256];
	wb_str_replace(png_path, file_name, ".fnt", "_0.png");

    image_t font_image = {0};
    image_load(png_path, &font_image);

    //font->texture_index = renderer_create_texture(&font_image).index;
}