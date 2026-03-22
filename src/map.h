#pragma once

#include <stdbool.h>
#include <stdint.h>

// Terrain types matching WBC3 (Data.cpp g_TerrainCost ordering)
enum
{
	TERRAIN_GRASS      = 0,
	TERRAIN_DIRT       = 1,
	TERRAIN_SAND       = 2,
	TERRAIN_WATER      = 3,
	TERRAIN_ROCK       = 4,
	TERRAIN_LAVA       = 5,
	TERRAIN_MARSH      = 6,
	TERRAIN_FORD       = 7,
	TERRAIN_SNOW       = 8,
	TERRAIN_VOID       = 9,
	TERRAIN_MOUNTAIN   = 10,
	TERRAIN_WALLS      = 11,
	TERRAIN_IMPASSABLE = 12,
	TERRAIN_COUNT      = 13,
};

// Movement modes matching WBC3 g_TerrainCost rows
enum
{
	MOVE_LAND  = 0,
	MOVE_FLY   = 1,
	MOVE_SEA   = 2,
	MOVE_FLOAT = 3,
	MOVE_COUNT = 4,
};

// Cell display size in pixels (matching WBC3: TILE_WIDTH=32, TILE_HEIGHT=24)
enum
{
	CELL_W = 32,
	CELL_H = 24,
};

// Cell flags (stored in map_cell.flags)
enum
{
	CELL_FLAG_BUILDING = 0x01,
	CELL_FLAG_MINE     = 0x02,
};

typedef struct
{
	uint8_t terrain; // TERRAIN_* enum
	uint8_t height;
	uint8_t flags;
} map_cell;

typedef struct
{
	map_cell* cells; // width * height, row-major
	uint32_t width;
	uint32_t height;
} game_map;

bool     map_init(game_map* map, uint32_t width, uint32_t height);
void     map_free(game_map* map);

map_cell map_get(const game_map* map, uint32_t x, uint32_t y);
void     map_set(game_map* map, uint32_t x, uint32_t y, map_cell cell);

bool     map_walkable(const game_map* map, uint32_t x, uint32_t y, uint8_t move_mode);
uint16_t map_cost(const game_map* map, uint32_t x, uint32_t y, uint8_t move_mode);

// Mark/unmark a rectangular region of cells with a flag
void     map_set_footprint(game_map* map, uint32_t cx, uint32_t cy, uint32_t w, uint32_t h, uint8_t flag);
void     map_clear_footprint(game_map* map, uint32_t cx, uint32_t cy, uint32_t w, uint32_t h, uint8_t flag);

// Check if a rectangular region is clear (walkable + no flags set)
bool     map_area_clear(const game_map* map, uint32_t cx, uint32_t cy, uint32_t w, uint32_t h, uint8_t move_mode);

// Generate a 128x128 procedural test map with varied terrain
void     map_generate_test(game_map* map);
