#ifndef SERVER
#pragma once

#include "../../../include/engine/types.h"

enum
{
    MAX_FONT_GLYPHS = 256,
    MAX_FONT_KERNINGS = 32
};

typedef struct kerning_t
{
    uint16_t previous_char;
    int16_t offset;
} kerning_t;

typedef struct glyph_t
{
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    int16_t x_offset;
    int16_t y_offset;
    uint16_t advance;
    uint16_t _pad;
} glyph_t;

_Static_assert(sizeof(glyph_t) == 16, "sizeof(glyph_t) must be 16.");

typedef struct font_t
{
    uint16_t texture_index;
    uint16_t glyph_count;
    glyph_t glyphs[MAX_FONT_GLYPHS];
    kerning_t kernings[MAX_FONT_KERNINGS];
} font_t;

void font_load(const char* name, font_t* font);
#endif