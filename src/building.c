#include "building.h"
#include "gfx.h"

#include <stdio.h>
#include <string.h>

void building_types_init_defaults(building_type types[BTYPE_COUNT])
{
	memset(types, 0, sizeof(building_type) * BTYPE_COUNT);

	// Town Hall: free starting building, generates gold
	types[BTYPE_TOWN_HALL] = (building_type){
		.name = "Town Hall",
		.cost = {0, 0, 0, 0},
		.build_time = 0.0f, // instant
		.max_health = 500,
		.footprint_w = 2,
		.footprint_h = 2,
		.gold_income = 5, // per tick (5 sec interval)
		.producible_count = 0,
	};

	// Barracks: produces basic melee unit
	types[BTYPE_BARRACKS] = (building_type){
		.name = "Barracks",
		.cost = {200, 100, 0, 50},
		.build_time = 15.0f,
		.max_health = 300,
		.footprint_w = 2,
		.footprint_h = 2,
		.gold_income = 0,
		.producible_count = 1,
		.producible = {0}, // unit_type index 0
		.produce_cost = {{50, 0, 0, 0}},
		.produce_time = {10.0f},
	};
}

void building_array_init(building_array* arr)
{
	arr->count = 0;
}

uint32_t building_place(building_array* arr, game_map* map, resource_bank* bank,
                        const building_type* types, uint8_t type_index,
                        uint32_t cx, uint32_t cy, uint8_t team)
{
	if (arr->count >= MAX_BUILDINGS || type_index >= BTYPE_COUNT)
		return UINT32_MAX;

	const building_type* bt = &types[type_index];

	// Check area clear
	if (!map_area_clear(map, cx, cy, bt->footprint_w, bt->footprint_h, MOVE_LAND))
		return UINT32_MAX;

	// Check affordability
	if (bank && !resource_can_afford(bank, bt->cost))
		return UINT32_MAX;

	// Deduct cost
	if (bank)
		resource_deduct(bank, bt->cost);

	// Mark map cells
	map_set_footprint(map, cx, cy, bt->footprint_w, bt->footprint_h, CELL_FLAG_BUILDING);

	// Create building
	uint32_t idx = arr->count++;
	building* b = &arr->buildings[idx];
	memset(b, 0, sizeof(*b));
	b->cx = cx;
	b->cy = cy;
	b->type_index = type_index;
	b->team = team;
	b->health = bt->max_health;
	b->produce_index = -1;

	if (bt->build_time <= 0.0f)
	{
		b->state = BSTATE_ACTIVE;
		b->build_progress = 0.0f;
	}
	else
	{
		b->state = BSTATE_CONSTRUCTING;
		b->build_progress = 0.0f;
	}

	fprintf(stderr, "[building] Placed %s at (%u,%u) for team %u\n", bt->name, cx, cy, team);
	return idx;
}

// Find nearest walkable cell adjacent to the building footprint for unit spawning
static bool find_spawn_point(const game_map* map, uint32_t cx, uint32_t cy,
                             uint32_t fw, uint32_t fh, float* out_x, float* out_y)
{
	// Search perimeter: bottom edge first, then sides, then top
	int dirs[][2] = {
		{0, 1}, {1, 1}, {-1, 1}, {1, 0}, {-1, 0}, {0, -1}, {1, -1}, {-1, -1}
	};

	for (int d = 0; d < 8; d++)
	{
		for (uint32_t i = 0; i < fw; i++)
		{
			int px = (int)cx + (int)i + (dirs[d][0] > 0 ? (int)fw : (dirs[d][0] < 0 ? -1 : 0));
			int py = (int)cy + (int)i + (dirs[d][1] > 0 ? (int)fh : (dirs[d][1] < 0 ? -1 : 0));
			if (px < 0 || py < 0) continue;
			if (map_walkable(map, (uint32_t)px, (uint32_t)py, MOVE_LAND))
			{
				*out_x = (float)px * CELL_W + CELL_W * 0.5f;
				*out_y = (float)py * CELL_H + CELL_H * 0.5f;
				return true;
			}
		}
	}

	// Fallback: just south of building center
	*out_x = (float)(cx * CELL_W + fw * CELL_W / 2);
	*out_y = (float)((cy + fh) * CELL_H + CELL_H / 2);
	return true;
}

void buildings_update(building_array* arr, const building_type* types,
                      resource_bank* banks, unit_array* units,
                      const unit_type* unit_types, uint32_t num_unit_types,
                      game_map* map, mine_array* mines, float dt)
{
	for (uint32_t i = 0; i < arr->count; i++)
	{
		building* b = &arr->buildings[i];
		const building_type* bt = &types[b->type_index];

		if (b->state == BSTATE_CONSTRUCTING)
		{
			b->build_progress += dt;
			if (b->build_progress >= bt->build_time)
			{
				b->state = BSTATE_ACTIVE;
				fprintf(stderr, "[building] %s construction complete\n", bt->name);

				// Claim nearby mines
				if (mines)
				{
					mines_claim_nearby(mines,
					                   b->cx + bt->footprint_w / 2,
					                   b->cy + bt->footprint_h / 2,
					                   10, b->team);
				}
			}
		}
		else if (b->state == BSTATE_ACTIVE)
		{
			// Passive income (town hall)
			if (bt->gold_income > 0)
			{
				// Reuse tick timer via build_progress field (repurposed after construction)
				b->build_progress += dt;
				if (b->build_progress >= 5.0f)
				{
					b->build_progress -= 5.0f;
					resource_add(&banks[b->team], RES_GOLD, bt->gold_income);
				}
			}

			// Unit production
			if (b->produce_index >= 0 && b->produce_index < bt->producible_count)
			{
				b->produce_progress += dt;
				if (b->produce_progress >= bt->produce_time[b->produce_index])
				{
					// Spawn unit
					uint8_t ut_idx = bt->producible[b->produce_index];
					if (ut_idx < num_unit_types)
					{
						float sx, sy;
						find_spawn_point(map, b->cx, b->cy, bt->footprint_w, bt->footprint_h, &sx, &sy);
						unit_spawn(units, &unit_types[ut_idx], sx, sy, b->team);
						fprintf(stderr, "[building] %s produced unit at (%.0f, %.0f)\n", bt->name, sx, sy);
					}
					b->produce_index = -1;
					b->produce_progress = 0.0f;
				}
			}
		}
	}
}

bool building_start_production(building* b, const building_type* bt,
                               resource_bank* bank, uint8_t produce_index)
{
	if (b->state != BSTATE_ACTIVE) return false;
	if (b->produce_index >= 0) return false; // already producing
	if (produce_index >= bt->producible_count) return false;
	if (!resource_can_afford(bank, bt->produce_cost[produce_index])) return false;

	resource_deduct(bank, bt->produce_cost[produce_index]);
	b->produce_index = (int8_t)produce_index;
	b->produce_progress = 0.0f;
	return true;
}

void buildings_draw(const building_array* arr, const building_type* types,
                    uint32_t white_tex)
{
	static const uint32_t team_colors[] = {0xFF4080FF, 0xFF4040FF, 0xFF40FF40, 0xFFFF4040};

	for (uint32_t i = 0; i < arr->count; i++)
	{
		const building* b = &arr->buildings[i];
		if (b->state == BSTATE_DESTROYED) continue;

		const building_type* bt = &types[b->type_index];
		float x = (float)(b->cx * CELL_W);
		float y = (float)(b->cy * CELL_H);
		float w = (float)(bt->footprint_w * CELL_W);
		float h = (float)(bt->footprint_h * CELL_H);

		uint32_t color = team_colors[b->team % 4];

		// Constructing: translucent
		if (b->state == BSTATE_CONSTRUCTING)
			color = (color & 0x00FFFFFF) | 0x80000000;

		// Selection highlight
		if (b->selected)
			color = 0xFF80FF80;

		// Building body
		gfx_draw_sprite(x + 2.0f, y + 2.0f, w - 4.0f, h - 4.0f, white_tex, color);

		// Border
		float t = 1.0f;
		uint32_t border = 0xFF000000;
		gfx_draw_sprite(x, y, w, t, white_tex, border);
		gfx_draw_sprite(x, y + h - t, w, t, white_tex, border);
		gfx_draw_sprite(x, y, t, h, white_tex, border);
		gfx_draw_sprite(x + w - t, y, t, h, white_tex, border);

		// HP bar (above building, only when damaged)
		if (b->health < bt->max_health && b->health > 0)
		{
			float bar_w = w;
			float bar_h = 3.0f;
			float bar_x = x;
			float bar_y = y - bar_h - 2.0f;
			float hp_frac = (float)b->health / (float)bt->max_health;
			gfx_draw_sprite(bar_x, bar_y, bar_w, bar_h, white_tex, 0xFF0000FF);
			gfx_draw_sprite(bar_x, bar_y, bar_w * hp_frac, bar_h, white_tex, 0xFF00FF00);
		}

		// Construction progress bar
		if (b->state == BSTATE_CONSTRUCTING)
		{
			float bar_w = w;
			float bar_h = 3.0f;
			float bar_x = x;
			float bar_y = y - bar_h - 2.0f;
			float frac = b->build_progress / bt->build_time;
			gfx_draw_sprite(bar_x, bar_y, bar_w, bar_h, white_tex, 0xFF404040);
			gfx_draw_sprite(bar_x, bar_y, bar_w * frac, bar_h, white_tex, 0xFFFFFF00);
		}

		// Production progress bar
		if (b->state == BSTATE_ACTIVE && b->produce_index >= 0)
		{
			float bar_w = w - 8.0f;
			float bar_h = 3.0f;
			float bar_x = x + 4.0f;
			float bar_y = y + h + 2.0f;
			float frac = b->produce_progress / bt->produce_time[b->produce_index];
			gfx_draw_sprite(bar_x, bar_y, bar_w, bar_h, white_tex, 0xFF404040);
			gfx_draw_sprite(bar_x, bar_y, bar_w * frac, bar_h, white_tex, 0xFF00FFFF);
		}
	}
}

bool building_damage(building* b, building_array* arr, game_map* map,
                     const building_type* types, int16_t damage)
{
	(void)arr;
	b->health -= damage;
	if (b->health <= 0)
	{
		const building_type* bt = &types[b->type_index];
		map_clear_footprint(map, b->cx, b->cy, bt->footprint_w, bt->footprint_h, CELL_FLAG_BUILDING);
		b->state = BSTATE_DESTROYED;
		fprintf(stderr, "[building] %s destroyed\n", bt->name);
		return true;
	}
	return false;
}

int32_t building_find_at(const building_array* arr, const building_type* types,
                         uint32_t cx, uint32_t cy)
{
	for (uint32_t i = 0; i < arr->count; i++)
	{
		const building* b = &arr->buildings[i];
		if (b->state == BSTATE_DESTROYED) continue;
		const building_type* bt = &types[b->type_index];
		if (cx >= b->cx && cx < b->cx + bt->footprint_w &&
		    cy >= b->cy && cy < b->cy + bt->footprint_h)
			return (int32_t)i;
	}
	return -1;
}
