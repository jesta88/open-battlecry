#pragma once

#include "font.h"
#include "gfx.h"
#include <stdint.h>

typedef enum
{
	GAME_STATE_MAIN_MENU = 0,
	GAME_STATE_PLAYING   = 1,
	GAME_STATE_PAUSED    = 2,
} game_state;

typedef enum
{
	MENU_ACTION_NONE   = 0,
	MENU_ACTION_PLAY   = 1,
	MENU_ACTION_QUIT   = 2,
	MENU_ACTION_RESUME = 3,
} menu_action;

typedef struct
{
	const font* ui_font;
	const font* title_font;
	uint32_t white_tex;
} menu_config;

void        menu_init(const menu_config* config);
menu_action menu_update(game_state state, const gfx_input* input);
void        menu_draw(game_state state, uint32_t screen_w, uint32_t screen_h);
