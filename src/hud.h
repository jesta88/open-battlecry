#pragma once

#include "font.h"
#include "entity.h"
#include "resource.h"
#include <stdint.h>

typedef struct
{
	const font* ui_font;
	uint32_t white_tex;
} hud_config;

void hud_init(const hud_config* config);
void hud_draw(const unit_array* units, const resource_bank* player_bank,
              float dt, uint32_t screen_w, uint32_t screen_h);
