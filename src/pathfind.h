#pragma once

#include "map.h"
#include <stdbool.h>
#include <stdint.h>

enum
{
	PATHFIND_MAX_PATH = 512,
};

typedef struct
{
	uint16_t x, y;
} path_point;

typedef struct
{
	path_point points[PATHFIND_MAX_PATH];
	uint16_t length;  // total points in path
	uint16_t current; // next point to walk toward
} path_result;

// Find a path from (sx,sy) to (dx,dy) on the given map.
// move_mode: MOVE_LAND, MOVE_FLY, etc.
// Returns true if a path was found. Result written to *out.
bool pathfind_find(const game_map* map, uint16_t sx, uint16_t sy,
                   uint16_t dx, uint16_t dy, uint8_t move_mode,
                   path_result* out);
