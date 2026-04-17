#pragma once

// SCN scenario file parser + TER tile type parser.
// Compiled into asset_baker only (like xcr.c).

#include <stdbool.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// Tile type catalog (loaded from TER resources in terrain XCR archives)
// ---------------------------------------------------------------------------

#define SCN_MAX_CELLS_PER_TILE 20
#define SCN_BASE_CELLS_PER_TILE 10

typedef struct
{
	uint32_t id;                                            // 4-char tile ID (e.g. "GRSA")
	uint8_t  terrain[SCN_MAX_CELLS_PER_TILE][SCN_MAX_CELLS_PER_TILE]; // bTerrainType per cell
	uint8_t  height[SCN_MAX_CELLS_PER_TILE][SCN_MAX_CELLS_PER_TILE];  // bHeight per cell
	uint8_t  cell_w, cell_h;                                // actual cell dimensions based on bSize
} scn_tile_type;

typedef struct
{
	scn_tile_type* types;
	uint32_t count;
	uint32_t capacity;
} scn_tile_catalog;

// ---------------------------------------------------------------------------
// Parsed scenario data
// ---------------------------------------------------------------------------

// Tile placement from MAP chunk
typedef struct
{
	uint32_t id;   // tile ID reference
	uint8_t  part; // quadrant (0-3) for multi-position tiles
	uint8_t  size; // tile size class: 0=10x10, 1=20x20, 2=20x10, 3=10x20
} scn_tile_ref;

// Feature from FEATURE chunk
typedef struct
{
	uint32_t code; // feature type code
	int32_t  x, y; // cell position
	int32_t  direction;
} scn_feature;

// Side configuration from SIDE chunk
typedef struct
{
	bool     used;
	uint16_t race;
	uint16_t color;
	uint16_t team;
	int32_t  start_x, start_y;
	bool     start_random;
	uint16_t ai_level;
	bool     must_be_human;
	bool     must_be_on;
} scn_side;

#define SCN_MAX_SIDES 6

// Full parsed scenario
typedef struct
{
	char     name[32];
	char     code[16];
	uint32_t width, height; // map dimensions in cells
	uint32_t num_players;

	// Tile grid
	uint32_t      tile_grid_w, tile_grid_h;
	scn_tile_ref* tile_grid; // [tile_grid_h * tile_grid_w], row-major

	// Features
	scn_feature* features;
	uint32_t     num_features;
	uint32_t     feature_cap;

	// Sides
	scn_side sides[SCN_MAX_SIDES];
} scn_scenario;

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------

// Tile catalog lifecycle
void scn_tile_catalog_init(scn_tile_catalog* cat);
void scn_tile_catalog_free(scn_tile_catalog* cat);

// Load all TER resources from a terrain XCR archive into the catalog
bool scn_load_tile_types(scn_tile_catalog* cat, const char* xcr_path);

// Parse a .SCN scenario file
bool scn_load_scenario(const char* path, scn_scenario* out);
void scn_free_scenario(scn_scenario* scn);

// Expand tile grid to per-cell terrain/height arrays using the tile catalog.
// terrain_out and height_out must be pre-allocated: width * height bytes each.
// Returns true if at least one tile was resolved.
bool scn_expand_terrain(const scn_scenario* scn, const scn_tile_catalog* cat,
                        uint8_t* terrain_out, uint8_t* height_out);

// Tile visual output for a scenario (built by scn_build_tile_visuals)
typedef struct
{
	uint16_t* cell_tile_index; // per-cell tile texture index (w*h), 0xFFFF = none
	uint8_t*  cell_local_x;   // per-cell local X within tile (w*h)
	uint8_t*  cell_local_y;   // per-cell local Y within tile (w*h)

	uint32_t* tile_ids;       // unique tile IDs used [num_tiles]
	uint8_t*  tile_cell_w;    // cell width per tile [num_tiles]
	uint8_t*  tile_cell_h;    // cell height per tile [num_tiles]
	uint32_t  num_tiles;      // number of unique tile textures
} scn_tile_visuals;

// Build per-cell tile visual references from the scenario's tile grid.
// The tile catalog is used to look up tile dimensions (cell_w/cell_h).
bool scn_build_tile_visuals(const scn_scenario* scn, const scn_tile_catalog* cat,
                            scn_tile_visuals* out);
void scn_free_tile_visuals(scn_tile_visuals* vis);
