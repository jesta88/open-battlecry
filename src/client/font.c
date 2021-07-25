#include "font.h"
#include "image.h"
#include "renderer.h"
#include "../base/log.h"
#include "../base/bits.inl"
#include "../base/string.inl"
#include <SDL2/SDL_render.h>
#include <assert.h>

enum
{
    MAX_FONT_NAME_LENGTH = 64,
};

typedef struct fnt_info_t
{
    int16_t font_size;
    bits8_t bit_field;
    uint8_t char_set;
    uint16_t stretch_height;
    uint8_t anti_aliasing;
    uint8_t padding_up;
    uint8_t padding_right;
    uint8_t padding_down;
    uint8_t padding_left;
    uint8_t spacing_horizontal;
    uint8_t spacing_vertical;
    uint8_t outline;
    char font_name[MAX_FONT_NAME_LENGTH];
} fnt_info_t;

typedef struct fnt_common_t
{
    uint16_t line_height;
    uint16_t base;
    uint16_t scale_width;
    uint16_t scale_height;
    uint16_t pages;
    bits8_t bit_field;
    uint8_t alpha_channel;
    uint8_t red_channel;
    uint8_t green_channel;
    uint8_t blue_channel;
} fnt_common_t;

typedef struct fnt_pages_t
{
    char page_names[4][MAX_FONT_NAME_LENGTH];
} fnt_pages_t;

typedef struct fnt_char_t
{
    uint32_t id;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    int16_t x_offset;
    int16_t y_offset;
    int16_t advance;
    uint8_t page;
    uint8_t channel;
} fnt_char_t;

typedef struct fnt_kerning_t
{
    uint32_t first;
    uint32_t second;
    int16_t amount;
} fnt_kerning_t;


void font_load(const char* file_name, font_t* font)
{
    FILE* fnt_file = fopen(file_name, "rb");
    if (!fnt_file)
    {
        log_error("Failed to open file: %s", file_name);
        return;
    }

    {
        char identifier[4];
        fread(identifier, 1, sizeof(identifier), fnt_file);
        if (identifier[0] != 66 || identifier[1] != 77 || identifier[2] != 70)
        {
            log_error("File is not a bmfont file: %s", file_name);
            return;
        }
    }
    {
        uint8_t block_type;
        while (fread(&block_type, sizeof(block_type), 1, fnt_file))
        {
            uint32_t block_size;
            fread(&block_size, sizeof(block_size), 1, fnt_file);
            if (block_type < 4)
            {
                fseek(fnt_file, (int) block_size, SEEK_CUR);
            }
            else
            {
                const uint32_t char_count = block_size / (uint32_t) sizeof(fnt_char_t);
                assert(char_count <= MAX_FONT_GLYPHS);

                font->glyph_count = char_count;

                fnt_char_t chars[MAX_FONT_GLYPHS];
                fread(chars, block_size, 1, fnt_file);

                for (uint32_t i = 0; i < char_count; i++)
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
    string_replace(png_path, file_name, ".fnt", "_0.png");

    image_t font_image = {0};
    image_load(png_path, &font_image);

    font->texture_index = renderer_create_texture(&font_image).index;
}

