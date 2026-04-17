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

typedef struct baked_unit_def
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

// ---------------------------------------------------------------------------
// Raw texture format (.tex) — replaces BMP for fast loading
// Layout: [baked_tex_header][width * height * 4 bytes RGBA pixels]
// ---------------------------------------------------------------------------

#define BAKED_TEX_MAGIC 0x52415754 // "RAWT"

typedef struct
{
	uint32_t magic;
	uint32_t width;
	uint32_t height;
	uint32_t reserved; // future use (compression flags, format, etc.)
} baked_tex_header;

// ---------------------------------------------------------------------------
// Unit definition (one per unit type across all sides)
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

// ---------------------------------------------------------------------------
// Scenario format (.bscn) — baked from WBC3 .SCN scenario files
// Layout: [baked_scenario_header]
//         [terrain: u8 × w×h]
//         [height: u8 × w×h]
//         [tile visuals: baked_cell_visual × w×h]
//         [tile catalog: baked_tile_entry × num_tile_textures]
//         [features: baked_scenario_feature × num_features]
//         [sides: baked_scenario_side × num_sides]
// ---------------------------------------------------------------------------

#define BAKED_SCENARIO_MAGIC   0x4E454353 // "SCEN"
#define BAKED_SCENARIO_VERSION 3          // v2: tile visuals, v3: full feature codes

typedef struct baked_scenario_header
{
	uint32_t magic;             // BAKED_SCENARIO_MAGIC
	uint32_t version;           // BAKED_SCENARIO_VERSION
	uint32_t width;             // map width in cells
	uint32_t height;            // map height in cells
	uint32_t num_features;      // decorative objects
	uint32_t num_sides;         // player slots (max 6)
	uint32_t num_tile_textures; // entries in tile texture catalog
	char     name[32];          // display name
	char     code[16];          // scenario identifier
	uint8_t  reserved[56];      // future: army/building/event data
} baked_scenario_header;        // 136 bytes (unchanged)

// Per-cell tile visual reference
typedef struct
{
	uint16_t tile_tex_index; // index into tile catalog (0xFFFF = no tile)
	uint8_t  local_x;       // cell position within tile (0-19)
	uint8_t  local_y;       // cell position within tile (0-19)
} baked_cell_visual;         // 4 bytes

// Tile texture catalog entry
typedef struct
{
	uint32_t tile_id; // 4-char tile ID (matches BMP resource name, e.g. "TGB0")
	uint8_t  cell_w;  // tile width in cells (10 or 20)
	uint8_t  cell_h;  // tile height in cells (10 or 20)
	uint8_t  reserved[2];
} baked_tile_entry;   // 8 bytes

typedef struct
{
	uint32_t code;      // full 4-byte feature code (e.g. "PLBT")
	uint16_t x, y;      // cell position
	uint8_t  direction;  // 0-7
	uint8_t  flags;      // walkable/flyable
	uint8_t  reserved[2];
} baked_scenario_feature;  // 12 bytes

typedef struct
{
	uint16_t race;          // race enum (0-15)
	uint16_t color;         // color ID
	uint16_t team;          // team assignment
	int16_t  start_x;      // start position (-1 = random)
	int16_t  start_y;
	uint8_t  ai_level;     // 0 = human slot, 1-7 = AI difficulty
	uint8_t  flags;         // must_be_human, race_locked, etc.
	uint8_t  reserved[4];
} baked_scenario_side;     // 16 bytes

// ---------------------------------------------------------------------------
// Feature type definitions (baked/data/features.bin)
// ---------------------------------------------------------------------------

#define BAKED_FEATURES_MAGIC 0x54414546 // "FEAT"

typedef struct
{
	uint32_t code;       // 4-char feature code (e.g. "PLBT")
	char     name[20];   // display name
	uint8_t  shape_w;    // footprint width in cells (1-3)
	uint8_t  shape_h;    // footprint height in cells (1-3)
	uint8_t  walkable;   // can walk over
	uint8_t  flyable;    // can fly over
	uint8_t  reserved[4];
} baked_feature_def;     // 32 bytes
