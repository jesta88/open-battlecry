#pragma once

#include "baked_format.h"
#include "entity.h"
#include "map.h"
#include <stdbool.h>
#include <stdint.h>

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

// Load a WAV sound from a baked file. Returns audio handle or UINT32_MAX.
uint32_t baked_load_sound(const char* wav_path);

// Scan a baked side directory for all unit codes with ANI files.
// Returns number of codes found. codes is filled with null-terminated strings.
uint32_t baked_scan_side_codes(const char* side_dir, char codes[][8], uint32_t max_codes);
