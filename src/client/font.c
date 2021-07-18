#include "font.h"
#include "image.h"
#include "renderer.h"
#include "../base/log.h"
#include "../base/bits.inl"
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

void font_load(const char* name, font_t* font)
{
    FILE* fnt_file = fopen("../assets/fonts/arial_32.fnt", "rb");
    if (!fnt_file)
    {
        log_error("Failed to open file: %s", name);
        return;
    }

    {
        char identifier[4];
        fread(identifier, 1, sizeof(identifier), fnt_file);
        if (identifier[0] != 66 || identifier[1] != 77 || identifier[2] != 70)
        {
            log_error("File is not a bmfont file: %s", name);
            return;
        }
    }
    {
        uint8_t block_type;
        while (fread(&block_type, sizeof(block_type), 1, fnt_file))
        {
            uint32_t block_size;
            fread(&block_size, sizeof(block_size), 1, fnt_file);

            switch (block_type)
            {
                case 1:
                {
                    fnt_info_t info;
                    fread(&info, block_size, 1, fnt_file);
                    break;
                }
                case 2:
                {
                    fnt_common_t common;
                    fread(&common, block_size, 1, fnt_file);
                    break;
                }
                case 3:
                {
                    fnt_pages_t pages;
                    fread(&pages, block_size, 1, fnt_file);
                    break;
                }
                case 4:
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
                    break;
                }
                case 5:
                {
                    break;
                }
                default:
                    log_error("Invalid block type.");
                    return;
            }
        }
    }
    fclose(fnt_file);

    image_t font_image = {0};
    image_load("../assets/fonts/arial_32_0.png", &font_image);

    font->texture_index = renderer_create_texture(&font_image).index;
}

