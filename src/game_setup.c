#include "game_setup.h"
#include "entity.h"
#include "baked_format.h"

#include <string.h>

// ---------------------------------------------------------------------------
// Side definitions
// ---------------------------------------------------------------------------
// Builder codes are best guesses from WBC3 data. The game_find_builder_type()
// function falls back to the cheapest unit if the specified code isn't found.

const side_def g_sides[SIDE_COUNT] = {
	[SIDE_KNIGHTS]      = {"knights",      "Knights",       "PE"},
	[SIDE_EMPIRE]       = {"empire",       "Empire",        "PE"},
	[SIDE_BARBARIANS]   = {"barbarians",   "Barbarians",    "BD"},
	[SIDE_DWARVES]      = {"dwarves",      "Dwarves",       "MR"},
	[SIDE_DARK_DWARVES] = {"darkdwarves",  "Dark Dwarves",  "MR"},
	[SIDE_HIGH_ELVES]   = {"highelves",    "High Elves",    "HA"},
	[SIDE_WOOD_ELVES]   = {"woodelves",    "Wood Elves",    "PX"},
	[SIDE_DARK_ELVES]   = {"darkelves",    "Dark Elves",    "AS"},
	[SIDE_ORCS]         = {"orcs",         "Orcs",          "GR"},
	[SIDE_MINOTAURS]    = {"minotaurs",    "Minotaurs",     "SL"},
	[SIDE_SSRATHI]      = {"ssrathi",      "Ssrathi",       "SN"},
	[SIDE_UNDEAD]       = {"undead",       "Undead",        "SK"},
	[SIDE_DAEMONS]      = {"daemons",      "Daemons",       "IM"},
	[SIDE_FEY]          = {"fey",          "Fey",           "FY"},
	[SIDE_SWARM]        = {"swarm",        "Swarm",         "LV"},
	[SIDE_PLAGUELORDS]  = {"plaguelords",  "Plague Lords",  "ZO"},
};

// ---------------------------------------------------------------------------
// Default game setup
// ---------------------------------------------------------------------------

game_setup game_setup_default(void)
{
	return (game_setup){
		.num_teams  = 2,
		.map_width  = 128,
		.map_height = 128,
		.teams = {
			{.side = SIDE_KNIGHTS, .start_cx = 4,  .start_cy = 8},
			{.side = SIDE_UNDEAD,  .start_cx = 60, .start_cy = 8},
		},
	};
}

// ---------------------------------------------------------------------------
// Side lookup
// ---------------------------------------------------------------------------

uint8_t side_find_by_name(const char* folder_name)
{
	for (uint8_t i = 0; i < SIDE_COUNT; i++)
		if (strcmp(g_sides[i].folder, folder_name) == 0)
			return i;
	return SIDE_COUNT;
}

// ---------------------------------------------------------------------------
// Builder unit type finder
// ---------------------------------------------------------------------------

int32_t game_find_builder_type(const unit_type* types, uint32_t type_count,
                               const baked_unit_def* defs, uint32_t num_defs,
                               uint8_t side)
{
	if (type_count == 0) return -1;

	const char* builder_code = g_sides[side].builder_code;

	// Try the designated builder code first
	for (uint32_t i = 0; i < type_count; i++)
		if (strcmp(types[i].base_code, builder_code) == 0)
			return (int32_t)i;

	// Fallback: find cheapest unit by gold cost
	int32_t best = 0;
	uint16_t min_cost = UINT16_MAX;
	for (uint32_t i = 0; i < type_count; i++)
	{
		for (uint32_t d = 0; d < num_defs; d++)
		{
			if (strcmp(types[i].base_code, defs[d].code) == 0)
			{
				if (defs[d].cost_gold < min_cost)
				{
					min_cost = defs[d].cost_gold;
					best = (int32_t)i;
				}
				break;
			}
		}
	}
	return best;
}
