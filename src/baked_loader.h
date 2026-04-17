#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct baked_unit_def baked_unit_def;
typedef struct baked_scenario_header baked_scenario_header;
typedef struct unit_type unit_type;
typedef struct game_map game_map;

// Load unit definitions from baked binary file.
// Caller must free(*out).
bool baked_load_units(const char* bin_path, baked_unit_def** out, uint32_t* count);

// Load terrain textures from baked BMP directory.
// Populates textures[TERRAIN_COUNT] with GPU texture indices.
bool baked_load_terrain(const char* terrain_dir, uint32_t textures[]);

// Load a unit type's sprites + ANI from a baked side directory.
// side_dir: e.g., "baked/sides/knights"
// code: e.g., "KN"
bool baked_load_unit_type(unit_type* ut, const char* side_dir, const char* code);

// Batch-load all unit types from a side directory using multithreaded I/O.
// Decodes all sprite sheets in parallel, then uploads textures on the calling thread.
// Returns number of unit types successfully loaded (with at least stand + walk animations).
uint32_t baked_load_side_units(unit_type* out_types, uint32_t max_types,
                               const char* side_dir,
                               const baked_unit_def* defs, uint32_t num_defs);

// Load a WAV sound from a baked file. Returns audio handle or UINT32_MAX.
uint32_t baked_load_sound(const char* wav_path);

// Loaded feature instance (for rendering)
typedef struct
{
	uint32_t code;          // feature type code
	uint16_t x, y;          // cell position
	uint8_t  direction;     // 0-7
	uint32_t texture;       // GPU texture handle (UINT32_MAX = not loaded)
	uint16_t sheet_w, sheet_h; // sprite sheet dimensions
	uint8_t  shape_w, shape_h; // footprint in cells
	uint8_t  walkable;
	// ANI data for positioning
	int16_t  origin_x, origin_y; // pivot point
	uint8_t  num_frames;
	uint16_t frame_w, frame_h;   // single frame dimensions
} loaded_feature;

// Loaded scenario visuals (tile textures + features)
typedef struct
{
	// Tile visuals
	uint32_t* tile_textures;     // GPU texture handles [num_tile_textures]
	uint16_t* cell_tile_index;   // per-cell tile tex index (w*h), 0xFFFF = none
	uint8_t*  cell_local_x;     // per-cell local X within tile (w*h)
	uint8_t*  cell_local_y;     // per-cell local Y within tile (w*h)
	uint8_t*  tile_cell_w;      // tile width in cells [num_tile_textures]
	uint8_t*  tile_cell_h;      // tile height in cells [num_tile_textures]
	uint32_t  num_tile_textures;
	uint32_t  width, height;     // map dimensions (for indexing)

	// Features
	loaded_feature* features;
	uint32_t        num_features;
} scenario_visuals;

// Load a baked scenario (.bscn) into a game map.
// Allocates the map cells via map_init and fills terrain + height data.
// If vis is non-NULL, loads tile visual data, tile textures, and features.
// Optionally returns the scenario header (pass NULL to skip).
bool baked_load_scenario(const char* bscn_path, const char* tiles_dir,
                         const char* features_dir, game_map* map,
                         scenario_visuals* vis, baked_scenario_header* hdr_out);

void scenario_visuals_free(scenario_visuals* vis);
