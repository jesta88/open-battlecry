#include "hud.h"
#include "gfx.h"

#include <stdio.h>
#include <string.h>

static struct
{
	const font* ui_font;
	uint32_t white_tex;
	float fps_accum;
	int fps_frames;
	float fps_display;
} s;

void hud_init(const hud_config* config)
{
	s.ui_font = config->ui_font;
	s.white_tex = config->white_tex;
	s.fps_display = 0.0f;
	s.fps_accum = 0.0f;
	s.fps_frames = 0;
}

// Draw a screen-fixed rectangle (cancels camera internally)
static void draw_screen_rect(float x, float y, float w, float h, uint32_t color)
{
	float cam_x, cam_y;
	gfx_get_camera(&cam_x, &cam_y);
	gfx_draw_sprite(x - cam_x, y - cam_y, w, h, s.white_tex, color);
}

void hud_draw(const unit_array* units, const resource_bank* player_bank,
              float dt, uint32_t screen_w, uint32_t screen_h)
{
	if (!s.ui_font) return;

	// FPS counter (top-right)
	s.fps_accum += dt;
	s.fps_frames++;
	if (s.fps_accum >= 0.5f)
	{
		s.fps_display = (float)s.fps_frames / s.fps_accum;
		s.fps_accum = 0.0f;
		s.fps_frames = 0;
	}

	char fps_buf[32];
	snprintf(fps_buf, sizeof(fps_buf), "FPS: %.0f", s.fps_display);
	float fps_w = font_measure_text(s.ui_font, fps_buf);
	draw_screen_rect((float)screen_w - fps_w - 16.0f, 4.0f, fps_w + 12.0f, s.ui_font->line_height + 4.0f, 0xA0000000);
	font_draw_text(s.ui_font, (float)screen_w - fps_w - 10.0f, 6.0f, fps_buf, 0xFFFFFFFF);

	// Resource bar (top)
	draw_screen_rect(0.0f, 0.0f, (float)screen_w, s.ui_font->line_height + 8.0f, 0xA0000000);
	{
		char res_buf[128];
		if (player_bank)
			snprintf(res_buf, sizeof(res_buf), "Gold: %d  |  Metal: %d  |  Crystal: %d  |  Stone: %d",
			         player_bank->amount[RES_GOLD], player_bank->amount[RES_METAL],
			         player_bank->amount[RES_CRYSTAL], player_bank->amount[RES_STONE]);
		else
			snprintf(res_buf, sizeof(res_buf), "Gold: 0  |  Metal: 0  |  Crystal: 0  |  Stone: 0");
		font_draw_text(s.ui_font, 10.0f, 4.0f, res_buf, 0xFFD4AF37);
	}

	// Unit info panel (bottom-left)
	uint32_t selected_count = 0;
	const unit* first_selected = NULL;
	for (uint32_t i = 0; i < units->count; i++)
	{
		const unit* u = &units->units[i];
		if (u->selected && u->team == 0 && u->state != UNIT_STATE_DYING)
		{
			selected_count++;
			if (!first_selected)
				first_selected = u;
		}
	}

	if (selected_count > 0)
	{
		float panel_w = 220.0f;
		float panel_h = s.ui_font->line_height * 3.0f + 16.0f;
		float panel_x = 8.0f;
		float panel_y = (float)screen_h - panel_h - 8.0f;

		draw_screen_rect(panel_x, panel_y, panel_w, panel_h, 0xC0000000);

		if (selected_count == 1 && first_selected)
		{
			char name_buf[64];
			snprintf(name_buf, sizeof(name_buf), "%s", first_selected->type->base_code);
			font_draw_text(s.ui_font, panel_x + 8.0f, panel_y + 6.0f, name_buf, 0xFFFFFFFF);

			char hp_buf[64];
			snprintf(hp_buf, sizeof(hp_buf), "HP: %d / %d", first_selected->health, first_selected->type->max_health);
			font_draw_text(s.ui_font, panel_x + 8.0f, panel_y + 6.0f + s.ui_font->line_height, hp_buf, 0xFF00FF00);

			// Health bar
			float bar_x = panel_x + 8.0f;
			float bar_y = panel_y + 6.0f + s.ui_font->line_height * 2.0f;
			float bar_w = panel_w - 16.0f;
			float bar_h = 8.0f;
			float hp_frac = (float)first_selected->health / (float)first_selected->type->max_health;
			if (hp_frac < 0.0f) hp_frac = 0.0f;
			draw_screen_rect(bar_x, bar_y, bar_w, bar_h, 0xFF000080);
			draw_screen_rect(bar_x, bar_y, bar_w * hp_frac, bar_h, 0xFF00FF00);
		}
		else
		{
			char sel_buf[64];
			snprintf(sel_buf, sizeof(sel_buf), "%u units selected", selected_count);
			font_draw_text(s.ui_font, panel_x + 8.0f, panel_y + 6.0f, sel_buf, 0xFFFFFFFF);
		}
	}
}
