#pragma once

// Shared binary format definitions for the asset baker and game loader.
// Both the baker tool and game executable include this header.
// Changes here require re-running the baker.

#include <stdint.h>

// ---------------------------------------------------------------------------
// File header (shared by all baked binary files)
// ---------------------------------------------------------------------------

#define BAKED_UNITS_MAGIC   0x554E4954 // "UNIT"
#define BAKED_BLDGS_MAGIC   0x424C4447 // "BLDG"
#define BAKED_VERSION       1

typedef struct
{
	uint32_t magic;
	uint32_t version;
	uint32_t record_count;
	uint32_t record_size; // sizeof(record struct), for validation
} baked_header;

// ---------------------------------------------------------------------------
// Unit definition (one per unit type across all sides)
// ---------------------------------------------------------------------------

typedef struct
{
	char code[8];      // base code, e.g. "KN" (null-padded)
	char name[32];     // display name, e.g. "Knight"
	char side[32];     // side name, e.g. "knights"

	// Classification
	uint8_t movement_type; // 0=land, 1=air, 2=sea, 3=hover
	uint8_t size_class;    // 0=small, 1=medium, 2=large, 3=huge
	uint8_t alignment;     // 0=good, 1=neutral, 2=evil
	uint8_t race;          // race index

	// Combat stats
	uint16_t hits;         // max HP
	uint16_t combat;       // combat skill
	uint16_t speed;        // movement speed
	uint16_t damage;       // attack damage
	uint16_t damage_range; // attack range
	uint16_t armor;        // armor value
	uint16_t resistance;   // magic resistance
	uint16_t view;         // sight range

	// Economy
	uint16_t cost_gold;
	uint16_t cost_metal;
	uint16_t cost_crystal;
	uint16_t cost_stone;
	uint16_t produce_time; // seconds to build

	// Magic
	uint16_t max_mana;
	uint16_t start_mana;

	// Flags
	uint8_t is_missile;
	uint8_t is_general;
	uint8_t pad[2];

	char missile_code[8]; // projectile unit code

	uint8_t reserved[18]; // future expansion
} baked_unit_def;

// ---------------------------------------------------------------------------
// Building definition (one per building type across all sides)
// ---------------------------------------------------------------------------

typedef struct
{
	char code[8];
	char name[32];
	char side[32];

	// Economy
	uint16_t cost_gold;
	uint16_t cost_metal;
	uint16_t cost_crystal;
	uint16_t cost_stone;
	uint16_t build_time; // seconds

	// Combat
	uint16_t hits;
	uint16_t damage;
	uint16_t damage_range;
	uint16_t armor;

	// Config
	uint8_t resource_type; // what resource it generates (RES_* or 0xFF=none)
	uint8_t footprint;     // 1=1x1, 2=2x2, 3=3x3
	uint8_t pad[2];

	uint8_t reserved[16];
} baked_building_def;
