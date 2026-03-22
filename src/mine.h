#pragma once

#include "map.h"
#include "resource.h"
#include <stdbool.h>
#include <stdint.h>

enum
{
	MAX_MINES = 32,
};

typedef struct
{
	uint32_t cx, cy;
	uint8_t resource_type; // RES_GOLD etc.
	uint8_t owner;         // team index, 0xFF = unclaimed
	int32_t yield_per_tick;
	float tick_timer;
	float tick_interval;
	bool active;
} mine;

typedef struct
{
	mine mines[MAX_MINES];
	uint32_t count;
} mine_array;

void     mine_array_init(mine_array* arr);
uint32_t mine_place(mine_array* arr, game_map* map, uint32_t cx, uint32_t cy,
                    uint8_t resource_type, int32_t yield);
void     mines_update(mine_array* arr, resource_bank* banks, float dt);
void     mines_draw(const mine_array* arr, uint32_t white_tex);
void     mines_claim_nearby(mine_array* arr, uint32_t cx, uint32_t cy, uint32_t radius, uint8_t team);
