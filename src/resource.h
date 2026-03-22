#pragma once

#include <stdbool.h>
#include <stdint.h>

enum
{
	RES_GOLD    = 0,
	RES_METAL   = 1,
	RES_CRYSTAL = 2,
	RES_STONE   = 3,
	RES_COUNT   = 4,
	MAX_TEAMS   = 4,
};

typedef struct
{
	int32_t amount[RES_COUNT];
} resource_bank;

typedef struct
{
	int32_t gold, metal, crystal, stone;
} resource_cost;

void resource_init(resource_bank* bank, int32_t gold, int32_t metal, int32_t crystal, int32_t stone);
bool resource_can_afford(const resource_bank* bank, resource_cost cost);
void resource_deduct(resource_bank* bank, resource_cost cost);
void resource_add(resource_bank* bank, uint8_t type, int32_t amount);
