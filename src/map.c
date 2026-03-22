#include "map.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Terrain cost table (from WBC3 Data.cpp g_TerrainCost)
// Indexed as [move_mode][terrain_type]. Values >= 1000 are impassable.
// ---------------------------------------------------------------------------

static const uint16_t g_terrain_cost[MOVE_COUNT][TERRAIN_COUNT] = {
	// grass dirt sand water rock lava marsh ford snow void mountain walls impassable
	{2, 2, 2, 1000, 2, 1000, 2, 2, 2, 1000, 1000, 2, 2000}, // LAND
	{2, 2, 2, 2, 2, 2, 2, 2, 2, 1000, 2, 2, 2000},           // FLY
	{1000, 1000, 1000, 2, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000}, // SEA
	{2, 2, 2, 2, 2, 1000, 2, 2, 2, 1000, 1000, 2, 2000},     // FLOAT
};

// ---------------------------------------------------------------------------
// Map lifecycle
// ---------------------------------------------------------------------------

bool map_init(game_map* map, uint32_t width, uint32_t height)
{
	map->width = width;
	map->height = height;
	map->cells = calloc((size_t)width * height, sizeof(map_cell));
	return map->cells != NULL;
}

void map_free(game_map* map)
{
	free(map->cells);
	map->cells = NULL;
	map->width = 0;
	map->height = 0;
}

// ---------------------------------------------------------------------------
// Cell access
// ---------------------------------------------------------------------------

map_cell map_get(const game_map* map, uint32_t x, uint32_t y)
{
	if (x >= map->width || y >= map->height)
		return (map_cell){.terrain = TERRAIN_VOID};
	return map->cells[y * map->width + x];
}

void map_set(game_map* map, uint32_t x, uint32_t y, map_cell cell)
{
	if (x >= map->width || y >= map->height)
		return;
	map->cells[y * map->width + x] = cell;
}

// ---------------------------------------------------------------------------
// Movement queries
// ---------------------------------------------------------------------------

bool map_walkable(const game_map* map, uint32_t x, uint32_t y, uint8_t move_mode)
{
	if (x >= map->width || y >= map->height)
		return false;
	if (map->cells[y * map->width + x].flags & CELL_FLAG_BUILDING)
		return false;
	return map_cost(map, x, y, move_mode) < 1000;
}

uint16_t map_cost(const game_map* map, uint32_t x, uint32_t y, uint8_t move_mode)
{
	if (x >= map->width || y >= map->height)
		return 2000;
	if (move_mode >= MOVE_COUNT)
		return 2000;
	map_cell c = map->cells[y * map->width + x];
	if (c.flags & CELL_FLAG_BUILDING)
		return 2000;
	if (c.terrain >= TERRAIN_COUNT)
		return 2000;
	return g_terrain_cost[move_mode][c.terrain];
}

void map_set_footprint(game_map* map, uint32_t cx, uint32_t cy, uint32_t w, uint32_t h, uint8_t flag)
{
	for (uint32_t y = cy; y < cy + h && y < map->height; y++)
		for (uint32_t x = cx; x < cx + w && x < map->width; x++)
			map->cells[y * map->width + x].flags |= flag;
}

void map_clear_footprint(game_map* map, uint32_t cx, uint32_t cy, uint32_t w, uint32_t h, uint8_t flag)
{
	for (uint32_t y = cy; y < cy + h && y < map->height; y++)
		for (uint32_t x = cx; x < cx + w && x < map->width; x++)
			map->cells[y * map->width + x].flags &= ~flag;
}

bool map_area_clear(const game_map* map, uint32_t cx, uint32_t cy, uint32_t w, uint32_t h, uint8_t move_mode)
{
	for (uint32_t y = cy; y < cy + h; y++)
		for (uint32_t x = cx; x < cx + w; x++)
			if (!map_walkable(map, x, y, move_mode))
				return false;
	return true;
}

// ---------------------------------------------------------------------------
// Simple hash for procedural generation
// ---------------------------------------------------------------------------

static uint32_t hash2d(uint32_t x, uint32_t y)
{
	uint32_t h = x * 374761393u + y * 668265263u;
	h = (h ^ (h >> 13)) * 1274126177u;
	return h ^ (h >> 16);
}

// ---------------------------------------------------------------------------
// Procedural test map
// ---------------------------------------------------------------------------

void map_generate_test(game_map* map)
{
	// Fill with grass
	for (uint32_t y = 0; y < map->height; y++)
		for (uint32_t x = 0; x < map->width; x++)
			map_set(map, x, y, (map_cell){.terrain = TERRAIN_GRASS});

	// River of water running vertically, with a slight curve
	for (uint32_t y = 0; y < map->height; y++)
	{
		int center = (int)(map->width / 2) + (int)((hash2d(0, y) % 5) - 2);
		for (int dx = -2; dx <= 2; dx++)
		{
			int rx = center + dx;
			if (rx >= 0 && rx < (int)map->width)
				map_set(map, (uint32_t)rx, y, (map_cell){.terrain = TERRAIN_WATER});
		}
	}

	// Ford (bridge) across the river at a few points
	uint32_t ford_y[] = {map->height / 4, map->height / 2, map->height * 3 / 4};
	for (int f = 0; f < 3; f++)
	{
		for (uint32_t x = 0; x < map->width; x++)
		{
			if (map_get(map, x, ford_y[f]).terrain == TERRAIN_WATER)
				map_set(map, x, ford_y[f], (map_cell){.terrain = TERRAIN_FORD});
		}
	}

	// Mountain clusters (3 clusters)
	uint32_t mx[] = {map->width / 4, map->width * 3 / 4, map->width / 2};
	uint32_t my[] = {map->height / 4, map->height / 4, map->height * 3 / 4};
	for (int c = 0; c < 3; c++)
	{
		for (int dy = -4; dy <= 4; dy++)
			for (int dx = -4; dx <= 4; dx++)
			{
				int px = (int)mx[c] + dx;
				int py = (int)my[c] + dy;
				if (px < 0 || py < 0 || px >= (int)map->width || py >= (int)map->height)
					continue;
				int dist2 = dx * dx + dy * dy;
				if (dist2 <= 12)
					map_set(map, (uint32_t)px, (uint32_t)py, (map_cell){.terrain = TERRAIN_MOUNTAIN, .height = 3});
				else if (dist2 <= 20)
					map_set(map, (uint32_t)px, (uint32_t)py, (map_cell){.terrain = TERRAIN_ROCK, .height = 1});
			}
	}

	// Scatter some sand patches
	for (uint32_t y = 0; y < map->height; y++)
		for (uint32_t x = 0; x < map->width; x++)
		{
			if (map_get(map, x, y).terrain != TERRAIN_GRASS)
				continue;
			uint32_t h = hash2d(x, y);
			if ((h & 0xFF) < 8)
				map_set(map, x, y, (map_cell){.terrain = TERRAIN_SAND});
			else if ((h & 0xFF) < 12)
				map_set(map, x, y, (map_cell){.terrain = TERRAIN_DIRT});
		}
}
