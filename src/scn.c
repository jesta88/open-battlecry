// SCN scenario file parser + TER tile type parser.
// Compiled into asset_baker only (like xcr.c).

#include "scn.h"
#include "xcr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// WBC3 chunk IDs (little-endian MAKEID)
// ---------------------------------------------------------------------------

#define MAKEID(a, b, c, d) \
	((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

#define ID_SCNH MAKEID('S', 'C', 'N', 'H')
#define ID_SCNU MAKEID('S', 'C', 'N', 'U')
#define ID_MAPH MAKEID('M', 'A', 'P', 'H')
#define ID_MAPU MAKEID('M', 'A', 'P', 'U')
#define ID_FTRH MAKEID('F', 'T', 'R', 'H')
#define ID_FTRU MAKEID('F', 'T', 'R', 'U')
#define ID_SIDH MAKEID('S', 'I', 'D', 'H')
#define ID_SIDU MAKEID('S', 'I', 'D', 'U')
#define ID_ENDX MAKEID('E', 'N', 'D', 'X')
#define ID_ENDU MAKEID('E', 'N', 'D', 'U')

// ---------------------------------------------------------------------------
// Binary read helpers (little-endian)
// ---------------------------------------------------------------------------

static uint32_t read_u32(const uint8_t* p) { return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24); }
static int32_t  read_i32(const uint8_t* p) { return (int32_t)read_u32(p); }
static uint16_t read_u16(const uint8_t* p) { return (uint16_t)(p[0] | (p[1] << 8)); }

// Copy a narrow (ANSI) string from buffer to dest, null-terminate
static void copy_narrow(char* dst, const uint8_t* src, int max_chars)
{
	memcpy(dst, src, max_chars);
	dst[max_chars - 1] = '\0';
}

// Copy a wide (UTF-16LE) string to narrow, taking low byte of each wchar
static void copy_wide_to_narrow(char* dst, const uint8_t* src, int max_chars)
{
	for (int i = 0; i < max_chars - 1; i++)
		dst[i] = (char)src[i * 2];
	dst[max_chars - 1] = '\0';
}

// ---------------------------------------------------------------------------
// TER resource parsing
// ---------------------------------------------------------------------------

// Raw xTILETYPE on disk (MSVC 32-bit, default packing):
//   offset 0:    idCode (4 bytes)
//   offset 4:    xCell[20][20] — 400 × {height, terrain, move_cost, flags} = 1600 bytes
//   offset 1604: fLoaded (1), fAnimated (1), bNumFrames (1), bSize (1)
//   offset 1608: nUsageCount (2), pad (2)
//   offset 1612: lAnimTimes[8] (32)
//   offset 1644: lSizeForCache (4)
//   offset 1648: pData (4, pointer on 32-bit)
//   Total: 1652 bytes
#define RAW_TILETYPE_SIZE 1652
#define RAW_CELL_OFFSET   4
#define RAW_BSIZE_OFFSET  1607

void scn_tile_catalog_init(scn_tile_catalog* cat)
{
	memset(cat, 0, sizeof(*cat));
}

void scn_tile_catalog_free(scn_tile_catalog* cat)
{
	free(cat->types);
	memset(cat, 0, sizeof(*cat));
}

static void catalog_ensure(scn_tile_catalog* cat)
{
	if (cat->count < cat->capacity) return;
	cat->capacity = cat->capacity ? cat->capacity * 2 : 256;
	cat->types    = realloc(cat->types, cat->capacity * sizeof(scn_tile_type));
}

bool scn_load_tile_types(scn_tile_catalog* cat, const char* xcr_path)
{
	xcr_archive* archive = xcr_load(xcr_path);
	if (!archive) return false;

	uint32_t loaded = 0;
	for (uint32_t i = 0; i < archive->resource_count; i++)
	{
		const xcr_resource* res = &archive->resources[i];
		if (res->type != XCR_RESOURCE_TER) continue;
		if (res->size < RAW_TILETYPE_SIZE) continue;

		const uint8_t* data = res->data;

		catalog_ensure(cat);
		scn_tile_type* tile = &cat->types[cat->count];
		memset(tile, 0, sizeof(*tile));

		// idCode at offset 0
		memcpy(&tile->id, data, 4);

		// xCell[20][20] at offset 4 — each xCELLDATA is {height, terrain, move_cost, flags} = 4 bytes
		// Array layout: xCell[x][y] in C, so x is outer index → x * 20 + y
		const uint8_t* cells = data + RAW_CELL_OFFSET;
		for (int x = 0; x < SCN_MAX_CELLS_PER_TILE; x++)
		{
			for (int y = 0; y < SCN_MAX_CELLS_PER_TILE; y++)
			{
				int cell_idx      = (x * SCN_MAX_CELLS_PER_TILE + y) * 4;
				tile->height[x][y]  = cells[cell_idx + 0];
				tile->terrain[x][y] = cells[cell_idx + 1];
			}
		}

		// bSize at offset 1607
		uint8_t bSize = data[RAW_BSIZE_OFFSET];
		tile->cell_w  = SCN_BASE_CELLS_PER_TILE;
		tile->cell_h  = SCN_BASE_CELLS_PER_TILE;
		switch (bSize)
		{
			case 1: tile->cell_w = 20; tile->cell_h = 20; break; // 640×480
			case 2: tile->cell_w = 20; tile->cell_h = 10; break; // 640×240
			case 3: tile->cell_w = 10; tile->cell_h = 20; break; // 320×480
			default: break;                                        // 0: 320×240 = 10×10
		}

		cat->count++;
		loaded++;
	}

	xcr_free(archive);
	return loaded > 0;
}

// ---------------------------------------------------------------------------
// SCN scenario file parsing
// ---------------------------------------------------------------------------

// ANSI SCENARIO_HEADER field offsets (relative to start of data after 8-byte {idCode, lSize})
#define ANSI_CODE_OFF    16 // szCode[16], 16 bytes
#define ANSI_NAME_OFF    32 // szName[32], 32 bytes
#define ANSI_NPLAYERS_OFF 64
#define ANSI_WIDTH_OFF   68
#define ANSI_HEIGHT_OFF  72
#define ANSI_HDR_MIN     76 // minimum bytes we need past the 8-byte header

// Unicode SCENARIO_HEADER: char arrays become wchar_t arrays (×2 size)
#define UNI_CODE_OFF     32 // szVersion=32 bytes, then szCode starts
#define UNI_NAME_OFF     64 // szCode=32 bytes (16 wchars), then szName
#define UNI_NPLAYERS_OFF 128 // szName=64 bytes (32 wchars), then iNumPlayers
#define UNI_WIDTH_OFF    132
#define UNI_HEIGHT_OFF   136
#define UNI_HDR_MIN      140

// xTILEDATA on disk: 16 bytes
//   offset 0: idTile (4)
//   offset 4: nTile (2)
//   offset 6: bPart (1)
//   offset 7: fAnimated (1)
//   offset 8: bCurrentFrame (1)
//   offset 9: bMaxFrames (1)
//   offset 10: bFlags (1)
//   offset 11: bSize (1)
//   offset 12: lNextFrameTime (4)
#define TILEDATA_SIZE 16

// FEATURE_RECORD on disk: 24 bytes
//   offset 0: idCode (4)
//   offset 4: iAnimation (4)
//   offset 8: iDirection (4)
//   offset 12: iFrame (4)
//   offset 16: iX (4)
//   offset 20: iY (4)
#define FEATURE_RECORD_SIZE 24

// xSIDE on disk (MSVC 32-bit default packing): 44 bytes
//   See detailed layout in plan. Key offsets:
#define XSIDE_SIZE       44
#define SIDE_FUSED       0
#define SIDE_NRACE       2
#define SIDE_NCOLOR      6
#define SIDE_NTEAM       8
#define SIDE_NAILEVEL    16
#define SIDE_START_X     20
#define SIDE_START_Y     24
#define SIDE_START_RND   28
#define SIDE_MUSTBEHUMAN 40
#define SIDE_MUSTBEON    41

static void features_grow(scn_scenario* scn)
{
	if (scn->num_features < scn->feature_cap) return;
	scn->feature_cap = scn->feature_cap ? scn->feature_cap * 2 : 64;
	scn->features    = realloc(scn->features, scn->feature_cap * sizeof(scn_feature));
}

bool scn_load_scenario(const char* path, scn_scenario* out)
{
	memset(out, 0, sizeof(*out));

	FILE* f = fopen(path, "rb");
	if (!f) return false;

	// --- Read SCENARIO_HEADER ---
	uint8_t hdr_prefix[8];
	if (fread(hdr_prefix, 1, 8, f) != 8) { fclose(f); return false; }

	uint32_t scn_id   = read_u32(hdr_prefix);
	uint32_t scn_size = read_u32(hdr_prefix + 4);

	bool is_scn = (scn_id == ID_SCNH || scn_id == ID_SCNU);
	if (!is_scn)
	{
		fprintf(stderr, "[scn] Not a scenario file: %s (magic=0x%08X)\n", path, scn_id);
		fclose(f);
		return false;
	}

	bool unicode = (scn_id == ID_SCNU);

	// Read remaining header bytes. lSize includes the 8-byte {idCode, lSize}.
	uint32_t hdr_data_size = scn_size - 8;
	uint8_t* hdr           = malloc(hdr_data_size);
	if (!hdr || fread(hdr, 1, hdr_data_size, f) != hdr_data_size)
	{
		free(hdr);
		fclose(f);
		return false;
	}

	// Extract fields based on format
	if (unicode)
	{
		if (hdr_data_size < UNI_HDR_MIN) { free(hdr); fclose(f); return false; }
		copy_wide_to_narrow(out->code, hdr + UNI_CODE_OFF, 16);
		copy_wide_to_narrow(out->name, hdr + UNI_NAME_OFF, 32);
		out->num_players = read_u32(hdr + UNI_NPLAYERS_OFF);
		out->width       = read_u32(hdr + UNI_WIDTH_OFF);
		out->height      = read_u32(hdr + UNI_HEIGHT_OFF);
	}
	else
	{
		if (hdr_data_size < ANSI_HDR_MIN) { free(hdr); fclose(f); return false; }
		copy_narrow(out->code, hdr + ANSI_CODE_OFF, 16);
		copy_narrow(out->name, hdr + ANSI_NAME_OFF, 32);
		out->num_players = read_u32(hdr + ANSI_NPLAYERS_OFF);
		out->width       = read_u32(hdr + ANSI_WIDTH_OFF);
		out->height      = read_u32(hdr + ANSI_HEIGHT_OFF);
	}
	free(hdr);

	if (out->width == 0 || out->height == 0 || out->width > 10000 || out->height > 10000)
	{
		fprintf(stderr, "[scn] Invalid map dimensions: %ux%u in %s\n", out->width, out->height, path);
		fclose(f);
		return false;
	}

	// Allocate tile grid
	out->tile_grid_w = (out->width - 1) / SCN_BASE_CELLS_PER_TILE + 1;
	out->tile_grid_h = (out->height - 1) / SCN_BASE_CELLS_PER_TILE + 1;
	out->tile_grid   = calloc(out->tile_grid_w * out->tile_grid_h, sizeof(scn_tile_ref));
	if (!out->tile_grid) { fclose(f); return false; }

	// --- Chunk loop ---
	uint8_t chunk_hdr[8];
	while (fread(chunk_hdr, 1, 8, f) == 8)
	{
		uint32_t chunk_id   = read_u32(chunk_hdr);
		uint32_t chunk_size = read_u32(chunk_hdr + 4);

		if (chunk_id == ID_ENDX || chunk_id == ID_ENDU) break;

		// MAP chunk: tile grid (y-outer, x-inner)
		if (chunk_id == ID_MAPH || chunk_id == ID_MAPU)
		{
			for (uint32_t y = 0; y < out->tile_grid_h; y++)
			{
				for (uint32_t x = 0; x < out->tile_grid_w; x++)
				{
					uint8_t td[TILEDATA_SIZE];
					if (fread(td, 1, TILEDATA_SIZE, f) != TILEDATA_SIZE) goto done;

					scn_tile_ref* ref = &out->tile_grid[y * out->tile_grid_w + x];
					ref->id   = read_u32(td);  // idTile
					ref->part = td[6];          // bPart
					ref->size = td[11];         // bSize
				}
			}
		}
		// FEATURE chunk: one FEATURE_RECORD per chunk
		else if (chunk_id == ID_FTRH || chunk_id == ID_FTRU)
		{
			uint8_t fr[FEATURE_RECORD_SIZE];
			if (fread(fr, 1, FEATURE_RECORD_SIZE, f) != FEATURE_RECORD_SIZE) goto done;

			features_grow(out);
			scn_feature* feat = &out->features[out->num_features++];
			feat->code      = read_u32(fr);
			feat->direction = read_i32(fr + 8);
			feat->x         = read_i32(fr + 16);
			feat->y         = read_i32(fr + 20);
		}
		// SIDE chunk: 6 xSIDE structs in one bulk read
		else if (chunk_id == ID_SIDH || chunk_id == ID_SIDU)
		{
			uint32_t data_size = chunk_size > 8 ? chunk_size - 8 : 0;
			if (data_size == 0) continue;

			uint8_t* side_data = malloc(data_size);
			if (!side_data || fread(side_data, 1, data_size, f) != data_size)
			{
				free(side_data);
				goto done;
			}

			// Compute per-side stride from actual data (robust to padding variations)
			uint32_t stride = data_size / SCN_MAX_SIDES;
			if (stride < SIDE_MUSTBEON + 1) stride = XSIDE_SIZE;

			for (int i = 0; i < SCN_MAX_SIDES && (uint32_t)((i + 1) * stride) <= data_size; i++)
			{
				const uint8_t* s = side_data + i * stride;
				scn_side* side   = &out->sides[i];

				side->used         = s[SIDE_FUSED] != 0;
				side->race         = read_u16(s + SIDE_NRACE);
				side->color        = read_u16(s + SIDE_NCOLOR);
				side->team         = read_u16(s + SIDE_NTEAM);
				side->ai_level     = read_u16(s + SIDE_NAILEVEL);
				side->start_x      = read_i32(s + SIDE_START_X);
				side->start_y      = read_i32(s + SIDE_START_Y);
				side->start_random = s[SIDE_START_RND] != 0;
				side->must_be_human = s[SIDE_MUSTBEHUMAN] != 0;
				side->must_be_on    = s[SIDE_MUSTBEON] != 0;
			}

			free(side_data);
		}
		// Unknown chunk: skip
		else
		{
			uint32_t skip = chunk_size > 8 ? chunk_size - 8 : 0;
			if (skip > 0) fseek(f, (long)skip, SEEK_CUR);
		}
	}

done:
	fclose(f);
	return out->width > 0 && out->height > 0 && out->tile_grid != NULL;
}

void scn_free_scenario(scn_scenario* scn)
{
	free(scn->tile_grid);
	free(scn->features);
	memset(scn, 0, sizeof(*scn));
}

// ---------------------------------------------------------------------------
// Terrain expansion: tile grid → per-cell terrain + height
// ---------------------------------------------------------------------------

static const scn_tile_type* find_tile(const scn_tile_catalog* cat, uint32_t id)
{
	for (uint32_t i = 0; i < cat->count; i++)
	{
		if (cat->types[i].id == id) return &cat->types[i];
	}
	return NULL;
}

bool scn_expand_terrain(const scn_scenario* scn, const scn_tile_catalog* cat,
                        uint8_t* terrain_out, uint8_t* height_out)
{
	uint32_t w = scn->width;
	uint32_t h = scn->height;

	// Default: grass, height 0
	memset(terrain_out, 0, w * h);
	memset(height_out, 0, w * h);

	uint32_t resolved = 0, missed = 0;

	for (uint32_t ty = 0; ty < scn->tile_grid_h; ty++)
	{
		for (uint32_t tx = 0; tx < scn->tile_grid_w; tx++)
		{
			const scn_tile_ref* ref = &scn->tile_grid[ty * scn->tile_grid_w + tx];

			// Skip non-origin parts of multi-tile spans
			if (ref->part != 0) continue;

			// Skip empty tile slots
			if (ref->id == 0) continue;

			const scn_tile_type* tile = find_tile(cat, ref->id);
			if (!tile)
			{
				missed++;
				continue;
			}

			// Cell range this tile covers
			uint32_t cell_w  = tile->cell_w;
			uint32_t cell_h  = tile->cell_h;
			uint32_t base_cx = tx * SCN_BASE_CELLS_PER_TILE;
			uint32_t base_cy = ty * SCN_BASE_CELLS_PER_TILE;

			for (uint32_t ly = 0; ly < cell_h; ly++)
			{
				for (uint32_t lx = 0; lx < cell_w; lx++)
				{
					uint32_t cx = base_cx + lx;
					uint32_t cy = base_cy + ly;
					if (cx >= w || cy >= h) continue;

					uint32_t idx      = cy * w + cx;
					terrain_out[idx]  = tile->terrain[lx][ly];
					height_out[idx]   = tile->height[lx][ly];
				}
			}

			resolved++;
		}
	}

	fprintf(stderr, "[scn] Terrain: %u tiles resolved, %u unresolved\n", resolved, missed);
	return resolved > 0;
}

// ---------------------------------------------------------------------------
// Tile visual expansion: tile grid → per-cell tile texture references
// ---------------------------------------------------------------------------

#define MAX_UNIQUE_TILES 4096

// Find or insert a tile ID in the unique list, return its index
static uint16_t find_or_add_tile_id(uint32_t* ids, uint8_t* cw, uint8_t* ch,
                                     uint32_t* count, uint32_t id,
                                     uint8_t cell_w, uint8_t cell_h)
{
	for (uint32_t i = 0; i < *count; i++)
		if (ids[i] == id) return (uint16_t)i;

	if (*count >= MAX_UNIQUE_TILES) return 0xFFFF;

	uint32_t idx = (*count)++;
	ids[idx] = id;
	cw[idx]  = cell_w;
	ch[idx]  = cell_h;
	return (uint16_t)idx;
}

bool scn_build_tile_visuals(const scn_scenario* scn, const scn_tile_catalog* cat,
                            scn_tile_visuals* out)
{
	memset(out, 0, sizeof(*out));

	uint32_t w = scn->width;
	uint32_t h = scn->height;
	size_t cell_count = (size_t)w * h;

	out->cell_tile_index = malloc(cell_count * sizeof(uint16_t));
	out->cell_local_x    = calloc(cell_count, 1);
	out->cell_local_y    = calloc(cell_count, 1);

	// Temporary buffers for building the unique tile list
	uint32_t* tmp_ids = malloc(MAX_UNIQUE_TILES * sizeof(uint32_t));
	uint8_t*  tmp_cw  = malloc(MAX_UNIQUE_TILES);
	uint8_t*  tmp_ch  = malloc(MAX_UNIQUE_TILES);
	uint32_t  tmp_count = 0;

	if (!out->cell_tile_index || !out->cell_local_x || !out->cell_local_y ||
	    !tmp_ids || !tmp_cw || !tmp_ch)
	{
		free(tmp_ids); free(tmp_cw); free(tmp_ch);
		scn_free_tile_visuals(out);
		return false;
	}

	// Default: no tile
	memset(out->cell_tile_index, 0xFF, cell_count * sizeof(uint16_t));

	for (uint32_t ty = 0; ty < scn->tile_grid_h; ty++)
	{
		for (uint32_t tx = 0; tx < scn->tile_grid_w; tx++)
		{
			const scn_tile_ref* ref = &scn->tile_grid[ty * scn->tile_grid_w + tx];
			if (ref->part != 0 || ref->id == 0) continue;

			const scn_tile_type* tile = find_tile(cat, ref->id);
			uint8_t cell_w = tile ? tile->cell_w : SCN_BASE_CELLS_PER_TILE;
			uint8_t cell_h = tile ? tile->cell_h : SCN_BASE_CELLS_PER_TILE;

			uint16_t tex_idx = find_or_add_tile_id(tmp_ids, tmp_cw, tmp_ch,
			                                        &tmp_count, ref->id,
			                                        cell_w, cell_h);

			uint32_t base_cx = tx * SCN_BASE_CELLS_PER_TILE;
			uint32_t base_cy = ty * SCN_BASE_CELLS_PER_TILE;

			for (uint32_t ly = 0; ly < cell_h; ly++)
			{
				for (uint32_t lx = 0; lx < cell_w; lx++)
				{
					uint32_t cx = base_cx + lx;
					uint32_t cy = base_cy + ly;
					if (cx >= w || cy >= h) continue;

					size_t idx = (size_t)cy * w + cx;
					out->cell_tile_index[idx] = tex_idx;
					out->cell_local_x[idx]    = (uint8_t)lx;
					out->cell_local_y[idx]     = (uint8_t)ly;
				}
			}
		}
	}

	// Copy unique tile list to output
	out->num_tiles  = tmp_count;
	out->tile_ids   = malloc(tmp_count * sizeof(uint32_t));
	out->tile_cell_w = malloc(tmp_count);
	out->tile_cell_h = malloc(tmp_count);
	memcpy(out->tile_ids, tmp_ids, tmp_count * sizeof(uint32_t));
	memcpy(out->tile_cell_w, tmp_cw, tmp_count);
	memcpy(out->tile_cell_h, tmp_ch, tmp_count);

	free(tmp_ids); free(tmp_cw); free(tmp_ch);

	fprintf(stderr, "[scn] Tile visuals: %u unique tile textures\n", tmp_count);
	return true;
}

void scn_free_tile_visuals(scn_tile_visuals* vis)
{
	free(vis->cell_tile_index);
	free(vis->cell_local_x);
	free(vis->cell_local_y);
	free(vis->tile_ids);
	free(vis->tile_cell_w);
	free(vis->tile_cell_h);
	memset(vis, 0, sizeof(*vis));
}
