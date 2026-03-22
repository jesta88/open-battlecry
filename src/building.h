#pragma once

#include "map.h"
#include "resource.h"
#include "entity.h"
#include "mine.h"
#include <stdbool.h>
#include <stdint.h>

enum
{
	BTYPE_TOWN_HALL = 0,
	BTYPE_BARRACKS  = 1,
	BTYPE_COUNT     = 2,

	BSTATE_CONSTRUCTING = 0,
	BSTATE_ACTIVE       = 1,
	BSTATE_DESTROYED    = 2,

	MAX_BUILDINGS  = 32,
	MAX_PRODUCIBLE = 4,
};

typedef struct
{
	const char* name;
	resource_cost cost;
	float build_time;
	int16_t max_health;
	uint8_t footprint_w;
	uint8_t footprint_h;
	int32_t gold_income;

	uint8_t producible_count;
	uint8_t producible[MAX_PRODUCIBLE]; // indices into unit_type array
	resource_cost produce_cost[MAX_PRODUCIBLE];
	float produce_time[MAX_PRODUCIBLE];
} building_type;

typedef struct
{
	uint32_t cx, cy;
	uint8_t type_index;
	uint8_t team;
	uint8_t state;
	int16_t health;
	float build_progress;
	int8_t produce_index;   // -1 = idle
	float produce_progress;
	bool selected;
} building;

typedef struct
{
	building buildings[MAX_BUILDINGS];
	uint32_t count;
} building_array;

void building_types_init_defaults(building_type types[BTYPE_COUNT]);
void building_array_init(building_array* arr);

// Place a building. Validates area clear and affordability.
uint32_t building_place(building_array* arr, game_map* map, resource_bank* bank,
                        const building_type* types, uint8_t type_index,
                        uint32_t cx, uint32_t cy, uint8_t team);

// Tick all buildings: construction, production, passive income.
void buildings_update(building_array* arr, const building_type* types,
                      resource_bank* banks, unit_array* units,
                      const unit_type* unit_types, uint32_t num_unit_types,
                      game_map* map, mine_array* mines, float dt);

// Start producing a unit from a building's catalog.
bool building_start_production(building* b, const building_type* bt,
                               resource_bank* bank, uint8_t produce_index);

// Render all buildings.
void buildings_draw(const building_array* arr, const building_type* types,
                    uint32_t white_tex);

// Damage a building. Returns true if destroyed.
bool building_damage(building* b, building_array* arr, game_map* map,
                     const building_type* types, int16_t damage);

// Find building at cell position. Returns index or -1.
int32_t building_find_at(const building_array* arr, const building_type* types,
                         uint32_t cx, uint32_t cy);
