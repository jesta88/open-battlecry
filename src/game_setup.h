#pragma once

#include <stdint.h>

typedef struct unit_type unit_type;
typedef struct baked_unit_def baked_unit_def;

// ---------------------------------------------------------------------------
// Playable sides (races)
// ---------------------------------------------------------------------------

enum
{
	SIDE_KNIGHTS = 0,
	SIDE_EMPIRE,
	SIDE_BARBARIANS,
	SIDE_DWARVES,
	SIDE_DARK_DWARVES,
	SIDE_HIGH_ELVES,
	SIDE_WOOD_ELVES,
	SIDE_DARK_ELVES,
	SIDE_ORCS,
	SIDE_MINOTAURS,
	SIDE_SSRATHI,
	SIDE_UNDEAD,
	SIDE_DAEMONS,
	SIDE_FEY,
	SIDE_SWARM,
	SIDE_PLAGUELORDS,
	SIDE_COUNT,
};

typedef struct
{
	const char* folder;       // baked directory name, e.g. "knights"
	const char* display_name; // e.g. "Knights"
	const char* builder_code; // builder unit code, e.g. "PE"
} side_def;

extern const side_def g_sides[SIDE_COUNT];

// ---------------------------------------------------------------------------
// Game session setup
// ---------------------------------------------------------------------------

#define GAME_MAX_TEAMS 2

typedef struct
{
	uint8_t  side;     // SIDE_* enum value
	uint16_t start_cx; // starting cell X (town hall placement)
	uint16_t start_cy; // starting cell Y
} team_setup;

typedef struct
{
	team_setup teams[GAME_MAX_TEAMS];
	uint32_t   num_teams;
	uint32_t   map_width;
	uint32_t   map_height;
} game_setup;

// Create a default 2-player game setup (Knights vs Undead).
game_setup game_setup_default(void);

// Find a side index by folder name. Returns SIDE_COUNT if not found.
uint8_t side_find_by_name(const char* folder_name);

// Find the builder unit type index within a loaded unit_type array.
// Tries the side's designated builder_code first, then falls back to the
// cheapest unit (by gold cost). Returns -1 if no units are loaded.
int32_t game_find_builder_type(const unit_type* types, uint32_t type_count,
                               const baked_unit_def* defs, uint32_t num_defs,
                               uint8_t side);
