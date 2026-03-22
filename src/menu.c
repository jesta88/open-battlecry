#include "menu.h"
#include "gfx.h"

#include <string.h>

static struct
{
	const font* ui_font;
	const font* title_font;
	uint32_t white_tex;
	int selected_item;
} s;

void menu_init(const menu_config* config)
{
	s.ui_font = config->ui_font;
	s.title_font = config->title_font;
	s.white_tex = config->white_tex;
	s.selected_item = 0;
}

// Draw a screen-fixed rectangle
static void draw_screen_rect(float x, float y, float w, float h, uint32_t color)
{
	float cam_x, cam_y;
	gfx_get_camera(&cam_x, &cam_y);
	gfx_draw_sprite(x - cam_x, y - cam_y, w, h, s.white_tex, color);
}

static void draw_menu_items(const char** items, int count, float cx, float start_y, uint32_t screen_w)
{
	const font* f = s.ui_font;
	for (int i = 0; i < count; i++)
	{
		uint32_t color = (i == s.selected_item) ? 0xFF00D4FF : 0xFFA0A0A0;
		float y = start_y + (float)i * (f->line_height + 8.0f);

		// Highlight bar for selected item
		if (i == s.selected_item)
		{
			float item_w = font_measure_text(f, items[i]);
			draw_screen_rect(cx - item_w * 0.5f - 10.0f, y - 2.0f,
			                 item_w + 20.0f, f->line_height + 4.0f, 0x40FFFFFF);
		}

		font_draw_text_centered(f, 0.0f, y, (float)screen_w, items[i], color);
	}
}

menu_action menu_update(game_state state, const gfx_input* input)
{
	int item_count = 0;
	if (state == GAME_STATE_MAIN_MENU) item_count = 2;
	else if (state == GAME_STATE_PAUSED) item_count = 2;
	else return MENU_ACTION_NONE;

	if (input->key_down_pressed)
		s.selected_item = (s.selected_item + 1) % item_count;
	if (input->key_up_pressed)
		s.selected_item = (s.selected_item - 1 + item_count) % item_count;

	if (input->key_enter_pressed)
	{
		if (state == GAME_STATE_MAIN_MENU)
		{
			if (s.selected_item == 0) return MENU_ACTION_PLAY;
			if (s.selected_item == 1) return MENU_ACTION_QUIT;
		}
		else if (state == GAME_STATE_PAUSED)
		{
			if (s.selected_item == 0) return MENU_ACTION_RESUME;
			if (s.selected_item == 1) return MENU_ACTION_QUIT;
		}
	}

	// ESC in pause menu = resume
	if (state == GAME_STATE_PAUSED && input->key_escape_pressed)
		return MENU_ACTION_RESUME;

	return MENU_ACTION_NONE;
}

void menu_draw(game_state state, uint32_t screen_w, uint32_t screen_h)
{
	if (!s.ui_font) return;

	const font* title_font = s.title_font ? s.title_font : s.ui_font;
	float cx = (float)screen_w * 0.5f;
	float cy = (float)screen_h * 0.5f;

	if (state == GAME_STATE_MAIN_MENU)
	{
		// Full-screen dark background
		draw_screen_rect(0, 0, (float)screen_w, (float)screen_h, 0xFF101018);

		// Title
		float title_y = cy - title_font->line_height * 3.0f;
		font_draw_text_centered(title_font, 0.0f, title_y, (float)screen_w,
		                        "OPEN BATTLECRY", 0xFFFFD700);

		// Menu items
		const char* items[] = {"Play", "Quit"};
		float items_y = cy - s.ui_font->line_height * 0.5f;
		draw_menu_items(items, 2, cx, items_y, screen_w);
	}
	else if (state == GAME_STATE_PAUSED)
	{
		// Semi-transparent overlay
		draw_screen_rect(0, 0, (float)screen_w, (float)screen_h, 0xC0000000);

		// Title
		float title_y = cy - title_font->line_height * 3.0f;
		font_draw_text_centered(title_font, 0.0f, title_y, (float)screen_w,
		                        "PAUSED", 0xFFFFFFFF);

		// Menu items
		const char* items[] = {"Resume", "Quit"};
		float items_y = cy - s.ui_font->line_height * 0.5f;
		draw_menu_items(items, 2, cx, items_y, screen_w);
	}
}
