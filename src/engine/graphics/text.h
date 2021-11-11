#ifndef SERVER
#pragma once

#include "../../../include/engine/types.h"

typedef struct font_t font_t;
typedef struct render_target_t render_target_t;

enum text_align
{
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT
};

void text_draw(const font_t* font, render_target_t* render_target, int16_t x, int16_t y, const char* format, ...);
void text_draw_color(const font_t* font, render_target_t* render_target, int16_t x, int16_t y, uint32_t color, const char* format, ...);
#endif