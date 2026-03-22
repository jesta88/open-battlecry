#pragma once

#include <stdbool.h>
#include <stdint.h>

enum
{
	FONT_FIRST_CHAR  = 32,  // ASCII space
	FONT_LAST_CHAR   = 126, // ASCII tilde
	FONT_GLYPH_COUNT = FONT_LAST_CHAR - FONT_FIRST_CHAR + 1,
};

typedef struct
{
	float u0, v0, u1, v1; // UV coords in atlas (normalized)
	float x_offset;        // pixels from cursor to left edge of glyph
	float y_offset;        // pixels from baseline to top of glyph
	float width;           // glyph pixel width
	float height;          // glyph pixel height
	float advance;         // horizontal advance in pixels
} font_glyph;

typedef struct
{
	uint32_t atlas_texture; // gfx bindless texture index
	float pixel_size;       // the font size this was baked at
	float ascent;           // pixels above baseline
	float descent;          // pixels below baseline (negative)
	float line_height;      // ascent - descent + line_gap
	font_glyph glyphs[FONT_GLYPH_COUNT];
} font;

// Load a TTF from file path and bake into a GPU atlas at the given pixel size.
bool font_load(font* f, const char* ttf_path, float pixel_size);

// Draw a string at screen position (x, y). Camera offset is canceled internally.
// color: packed RGBA (0xFFFFFFFF = white).
// Returns the X position after the last character.
float font_draw_text(const font* f, float x, float y, const char* text, uint32_t color);

// Measure text width in pixels without drawing.
float font_measure_text(const font* f, const char* text);

// Draw text centered horizontally within a given width.
float font_draw_text_centered(const font* f, float x, float y, float width, const char* text, uint32_t color);
