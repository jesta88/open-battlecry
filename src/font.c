#include "font.h"
#include "gfx.h"
#include "file_io.h"
#include "stb_truetype.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool font_load(font* f, const char* ttf_path, float pixel_size)
{
	memset(f, 0, sizeof(*f));
	f->atlas_texture = UINT32_MAX;

	// Read TTF file
	size_t ttf_size = 0;
	uint8_t* ttf_data = read_file_binary(ttf_path, &ttf_size);
	if (!ttf_data)
	{
		fprintf(stderr, "[font] Failed to read: %s\n", ttf_path);
		return false;
	}

	// Parse font info
	stbtt_fontinfo info;
	if (!stbtt_InitFont(&info, ttf_data, 0))
	{
		fprintf(stderr, "[font] Failed to parse TTF: %s\n", ttf_path);
		free(ttf_data);
		return false;
	}

	float scale = stbtt_ScaleForPixelHeight(&info, pixel_size);
	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);
	f->pixel_size = pixel_size;
	f->ascent = (float)ascent * scale;
	f->descent = (float)descent * scale;
	f->line_height = (float)(ascent - descent + line_gap) * scale;

	// Bake font atlas — try 512x512, then 1024x1024
	stbtt_bakedchar baked[FONT_GLYPH_COUNT];
	int atlas_w = 512, atlas_h = 512;
	uint8_t* alpha_bitmap = malloc((size_t)atlas_w * atlas_h);

	int result = stbtt_BakeFontBitmap(ttf_data, 0, pixel_size,
	                                   alpha_bitmap, atlas_w, atlas_h,
	                                   FONT_FIRST_CHAR, FONT_GLYPH_COUNT, baked);
	if (result <= 0)
	{
		// Not all glyphs fit — retry with larger atlas
		free(alpha_bitmap);
		atlas_w = 1024;
		atlas_h = 1024;
		alpha_bitmap = malloc((size_t)atlas_w * atlas_h);
		result = stbtt_BakeFontBitmap(ttf_data, 0, pixel_size,
		                               alpha_bitmap, atlas_w, atlas_h,
		                               FONT_FIRST_CHAR, FONT_GLYPH_COUNT, baked);
		if (result <= 0)
		{
			fprintf(stderr, "[font] Atlas too small for %.0fpx: %s\n", pixel_size, ttf_path);
			free(alpha_bitmap);
			free(ttf_data);
			return false;
		}
	}

	// Convert 1-channel alpha to RGBA: white RGB + alpha channel
	// This allows color tinting via the sprite shader's tex * color multiplication
	uint8_t* rgba = malloc((size_t)atlas_w * atlas_h * 4);
	for (int i = 0; i < atlas_w * atlas_h; i++)
	{
		rgba[i * 4 + 0] = 255;
		rgba[i * 4 + 1] = 255;
		rgba[i * 4 + 2] = 255;
		rgba[i * 4 + 3] = alpha_bitmap[i];
	}
	free(alpha_bitmap);
	free(ttf_data);

	// Upload to GPU
	f->atlas_texture = gfx_create_texture((uint32_t)atlas_w, (uint32_t)atlas_h, rgba);
	free(rgba);

	if (f->atlas_texture == UINT32_MAX)
	{
		fprintf(stderr, "[font] Failed to create atlas texture\n");
		return false;
	}

	// Extract per-glyph metrics
	for (int i = 0; i < FONT_GLYPH_COUNT; i++)
	{
		stbtt_bakedchar* bc = &baked[i];
		font_glyph* g = &f->glyphs[i];

		g->u0 = (float)bc->x0 / (float)atlas_w;
		g->v0 = (float)bc->y0 / (float)atlas_h;
		g->u1 = (float)bc->x1 / (float)atlas_w;
		g->v1 = (float)bc->y1 / (float)atlas_h;
		g->x_offset = bc->xoff;
		g->y_offset = bc->yoff;
		g->width = (float)(bc->x1 - bc->x0);
		g->height = (float)(bc->y1 - bc->y0);
		g->advance = bc->xadvance;
	}

	fprintf(stderr, "[font] Loaded %.0fpx font: %s (%dx%d atlas)\n",
	        pixel_size, ttf_path, atlas_w, atlas_h);
	return true;
}

float font_draw_text(const font* f, float x, float y, const char* text, uint32_t color)
{
	if (!text || f->atlas_texture == UINT32_MAX)
		return x;

	// Cancel camera so text is screen-fixed
	float cam_x, cam_y;
	gfx_get_camera(&cam_x, &cam_y);

	float cursor_x = x;
	for (const char* c = text; *c; c++)
	{
		int ch = (unsigned char)*c;
		if (ch < FONT_FIRST_CHAR || ch > FONT_LAST_CHAR)
		{
			// Unknown char — advance by space width
			cursor_x += f->glyphs[0].advance;
			continue;
		}

		const font_glyph* g = &f->glyphs[ch - FONT_FIRST_CHAR];
		if (g->width > 0 && g->height > 0)
		{
			float draw_x = cursor_x + g->x_offset - cam_x;
			float draw_y = y + f->ascent + g->y_offset - cam_y;

			gfx_draw_sprite_region(draw_x, draw_y, g->width, g->height,
			                       f->atlas_texture, color,
			                       g->u0, g->v0,
			                       g->u1 - g->u0, g->v1 - g->v0);
		}
		cursor_x += g->advance;
	}
	return cursor_x;
}

float font_measure_text(const font* f, const char* text)
{
	if (!text)
		return 0;

	float width = 0;
	for (const char* c = text; *c; c++)
	{
		int ch = (unsigned char)*c;
		if (ch < FONT_FIRST_CHAR || ch > FONT_LAST_CHAR)
			width += f->glyphs[0].advance;
		else
			width += f->glyphs[ch - FONT_FIRST_CHAR].advance;
	}
	return width;
}

float font_draw_text_centered(const font* f, float x, float y, float width, const char* text, uint32_t color)
{
	float text_width = font_measure_text(f, text);
	float offset = (width - text_width) * 0.5f;
	if (offset < 0) offset = 0;
	return font_draw_text(f, x + offset, y, text, color);
}
