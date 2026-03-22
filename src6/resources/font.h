#ifndef SERVER
#pragma once

#include "std.h"

enum
{
    MAX_FONT_GLYPHS = 256,
    MAX_FONT_KERNINGS = 32
};

typedef struct kerning_t
{
    u16 previous_char;
    s16 offset;
} kerning_t;

typedef struct glyph_t
{
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    s16 x_offset;
    s16 y_offset;
    u16 advance;
    u16 _pad;
} glyph_t;

_Static_assert(sizeof(glyph_t) == 16, "sizeof(glyph_t) must be 16.");

typedef struct font_t
{
    u16 texture_index;
    u16 glyph_count;
    glyph_t glyphs[MAX_FONT_GLYPHS];
    kerning_t kernings[MAX_FONT_KERNINGS];
} font_t;

void font_load(const char* name, font_t* font);
#endif