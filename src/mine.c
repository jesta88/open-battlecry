#include "mine.h"
#include "gfx.h"

#include <stdlib.h>

void mine_array_init(mine_array* arr)
{
	arr->count = 0;
}

uint32_t mine_place(mine_array* arr, game_map* map, uint32_t cx, uint32_t cy,
                    uint8_t resource_type, int32_t yield)
{
	if (arr->count >= MAX_MINES) return UINT32_MAX;
	if (cx >= map->width || cy >= map->height) return UINT32_MAX;

	uint32_t idx = arr->count++;
	mine* m = &arr->mines[idx];
	m->cx = cx;
	m->cy = cy;
	m->resource_type = resource_type;
	m->owner = 0xFF;
	m->yield_per_tick = yield;
	m->tick_timer = 0.0f;
	m->tick_interval = 5.0f;
	m->active = true;

	map_set_footprint(map, cx, cy, 1, 1, CELL_FLAG_MINE);
	return idx;
}

void mines_update(mine_array* arr, resource_bank* banks, float dt)
{
	for (uint32_t i = 0; i < arr->count; i++)
	{
		mine* m = &arr->mines[i];
		if (!m->active || m->owner == 0xFF) continue;

		m->tick_timer += dt;
		if (m->tick_timer >= m->tick_interval)
		{
			m->tick_timer -= m->tick_interval;
			resource_add(&banks[m->owner], m->resource_type, m->yield_per_tick);
		}
	}
}

void mines_draw(const mine_array* arr, uint32_t white_tex)
{
	static const uint32_t mine_colors[RES_COUNT] = {
		0xFFFFD700, // gold - yellow
		0xFFC0C0C0, // metal - silver
		0xFF00FFFF, // crystal - cyan
		0xFF8B7355, // stone - brown
	};

	for (uint32_t i = 0; i < arr->count; i++)
	{
		const mine* m = &arr->mines[i];
		if (!m->active) continue;

		uint32_t color = mine_colors[m->resource_type % RES_COUNT];
		// Dim unclaimed mines
		if (m->owner == 0xFF)
			color = (color & 0x00FFFFFF) | 0x80000000;

		float x = (float)(m->cx * CELL_W) + 4.0f;
		float y = (float)(m->cy * CELL_H) + 4.0f;
		float w = (float)CELL_W - 8.0f;
		float h = (float)CELL_H - 8.0f;

		// Diamond shape: draw rotated (just use a small centered square for now)
		gfx_draw_sprite(x, y, w, h, white_tex, color);
	}
}

void mines_claim_nearby(mine_array* arr, uint32_t cx, uint32_t cy, uint32_t radius, uint8_t team)
{
	for (uint32_t i = 0; i < arr->count; i++)
	{
		mine* m = &arr->mines[i];
		if (!m->active || m->owner != 0xFF) continue;

		int dx = (int)m->cx - (int)cx;
		int dy = (int)m->cy - (int)cy;
		if ((uint32_t)(abs(dx) + abs(dy)) <= radius)
		{
			m->owner = team;
		}
	}
}
