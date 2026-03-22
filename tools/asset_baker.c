#include "xcr.h"
#include "image.h"
#include "ani.h"
#include "file_io.h"
#include "baked_format.h"
#include "cfg_parse.h"
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <direct.h>
#include <ctype.h>

// ---------------------------------------------------------------------------
// Utilities
// ---------------------------------------------------------------------------

static void mkdirs(const char* path)
{
	char tmp[512];
	snprintf(tmp, sizeof(tmp), "%s", path);
	for (char* p = tmp + 1; *p; p++)
	{
		if (*p == '/' || *p == '\\')
		{
			*p = '\0';
			_mkdir(tmp);
			*p = '/';
		}
	}
	_mkdir(tmp);
}

static void to_lower(char* dst, const char* src, size_t max)
{
	size_t i = 0;
	for (; src[i] && i < max - 1; i++)
		dst[i] = (char)tolower((unsigned char)src[i]);
	dst[i] = '\0';
}

// Strip file extension from a filename
static void strip_ext(char* dst, const char* src, size_t max)
{
	snprintf(dst, max, "%s", src);
	char* dot = strrchr(dst, '.');
	if (dot) *dot = '\0';
}

// Write RGBA pixels as BMP file
static bool write_bmp(const char* path, uint32_t w, uint32_t h, const uint8_t* rgba)
{
	if (!stbi_write_bmp(path, (int)w, (int)h, 4, rgba))
	{
		fprintf(stderr, "[baker] Failed to write BMP: %s\n", path);
		return false;
	}
	return true;
}

// Write raw bytes to a file
static bool write_file(const char* path, const void* data, size_t size)
{
	FILE* f = fopen(path, "wb");
	if (!f) return false;
	size_t written = fwrite(data, 1, size, f);
	fclose(f);
	return written == size;
}

// ---------------------------------------------------------------------------
// Phase 1: Terrain extraction
// ---------------------------------------------------------------------------

static void bake_terrain(const char* wbc3_root, const char* output_dir)
{
	fprintf(stderr, "\n[baker] === Phase 1: Terrain ===\n");

	// Terrain XCR names that map to our terrain types
	// WBC3 Assets/Terrain/ has: Grass.xcr, Dirt.xcr, Desert.xcr (=sand), Water.xcr,
	// Rock.xcr, Lava.xcr, Marsh.xcr, Snow.xcr, Mountains.xcr, etc.
	static const struct { const char* xcr_name; const char* output_name; } terrain_map[] = {
		{"Grass.xcr",          "grass"},
		{"Dirt.xcr",           "dirt"},
		{"Desert.xcr",         "sand"},
		{"Water.xcr",          "water"},
		{"Rock.xcr",           "rock"},
		{"Lava.xcr",           "lava"},
		{"Marsh.xcr",          "marsh"},
		{"Snow.xcr",           "snow"},
		{"Mountains.xcr",      "mountain"},
	};

	char terrain_out[512];
	snprintf(terrain_out, sizeof(terrain_out), "%s/terrain", output_dir);
	mkdirs(terrain_out);

	uint32_t extracted = 0;
	for (int t = 0; t < (int)(sizeof(terrain_map) / sizeof(terrain_map[0])); t++)
	{
		char xcr_path[512];
		snprintf(xcr_path, sizeof(xcr_path), "%s/Assets/Terrain/%s", wbc3_root, terrain_map[t].xcr_name);

		xcr_archive* archive = xcr_load(xcr_path);
		if (!archive)
		{
			fprintf(stderr, "[baker] Terrain XCR not found: %s\n", xcr_path);
			continue;
		}

		// Extract first valid BMP or RLE resource as the terrain tile
		bool found = false;
		for (uint32_t i = 0; i < archive->resource_count && !found; i++)
		{
			const xcr_resource* res = &archive->resources[i];
			image img = {0};

			if (res->type == XCR_RESOURCE_BMP)
			{
				if (!image_decode_bmp(res->data, res->size, &img)) continue;
			}
			else if (res->type == XCR_RESOURCE_RLE)
			{
				if (!image_decode_rle(res->data, res->size, &img)) continue;
			}
			else continue;

			char bmp_path[512];
			snprintf(bmp_path, sizeof(bmp_path), "%s/%s.bmp", terrain_out, terrain_map[t].output_name);
			if (write_bmp(bmp_path, img.width, img.height, img.pixels))
			{
				fprintf(stderr, "[baker] Terrain: %s -> %s (%ux%u)\n",
				        terrain_map[t].xcr_name, bmp_path, img.width, img.height);
				extracted++;
				found = true;
			}
			image_free(&img);
		}

		xcr_free(archive);
	}

	fprintf(stderr, "[baker] Terrain: %u tiles extracted\n", extracted);
}

// ---------------------------------------------------------------------------
// Phase 2: Unit sprite extraction
// ---------------------------------------------------------------------------

static const char* anim_suffixes[] = {
	".RLE",   // ANI_STAND
	"W.RLE",  // ANI_WALK
	"F.RLE",  // ANI_FIGHT
	"D.RLE",  // ANI_DIE
	"A.RLE",  // ANI_AMBIENT
	"Z.RLE",  // ANI_SPECIAL
	"C.RLE",  // ANI_CONVERT
	"S.RLE",  // ANI_SPELL
	"I.RLE",  // ANI_INTERFACE
};

static const char* anim_suffix_out[] = {
	"",    // stand
	"W",   // walk
	"F",   // fight
	"D",   // die
	"A",   // ambient
	"Z",   // special
	"C",   // convert
	"S",   // spell
	"I",   // interface
};

static void bake_side_sprites(const char* wbc3_root, const char* output_dir,
                              const char* xcr_filename)
{
	char xcr_path[512];
	snprintf(xcr_path, sizeof(xcr_path), "%s/Assets/Sides/%s", wbc3_root, xcr_filename);

	xcr_archive* archive = xcr_load(xcr_path);
	if (!archive) return;

	// Derive side name from XCR filename (e.g., "Knights.xcr" -> "knights")
	char side_name[64];
	strip_ext(side_name, xcr_filename, sizeof(side_name));
	to_lower(side_name, side_name, sizeof(side_name));

	char side_out[512];
	snprintf(side_out, sizeof(side_out), "%s/sides/%s", output_dir, side_name);
	mkdirs(side_out);

	// Find all unique unit codes from ANI resources
	char codes[256][8];
	uint32_t code_count = 0;

	for (uint32_t i = 0; i < archive->resource_count; i++)
	{
		const xcr_resource* res = &archive->resources[i];
		if (res->type != XCR_RESOURCE_ANI) continue;

		char code[8] = {0};
		const char* dot = strstr(res->name, ".");
		if (!dot || dot == res->name) continue;
		size_t len = (size_t)(dot - res->name);
		if (len >= sizeof(code)) continue;
		memcpy(code, res->name, len);
		code[len] = '\0';

		// Skip duplicates
		bool dup = false;
		for (uint32_t j = 0; j < code_count; j++)
			if (strcmp(codes[j], code) == 0) { dup = true; break; }
		if (dup) continue;

		if (code_count < 256)
		{
			memcpy(codes[code_count], code, 8);
			code_count++;
		}
	}

	uint32_t sprites_extracted = 0;

	for (uint32_t c = 0; c < code_count; c++)
	{
		// Extract ANI metadata
		char ani_name[64];
		snprintf(ani_name, sizeof(ani_name), "%s.ANI", codes[c]);
		const xcr_resource* ani_res = xcr_find(archive, ani_name);
		if (ani_res)
		{
			char ani_path[512];
			snprintf(ani_path, sizeof(ani_path), "%s/%s.ani", side_out, codes[c]);
			write_file(ani_path, ani_res->data, ani_res->size);
		}

		// Extract each animation sprite sheet
		for (int a = 0; a < 9; a++)
		{
			char rle_name[64];
			snprintf(rle_name, sizeof(rle_name), "%s%s", codes[c], anim_suffixes[a]);
			const xcr_resource* rle_res = xcr_find(archive, rle_name);
			if (!rle_res) continue;

			image img = {0};
			if (!image_decode_rle(rle_res->data, rle_res->size, &img)) continue;

			char bmp_path[512];
			snprintf(bmp_path, sizeof(bmp_path), "%s/%s%s.bmp",
			         side_out, codes[c], anim_suffix_out[a]);
			if (write_bmp(bmp_path, img.width, img.height, img.pixels))
				sprites_extracted++;
			image_free(&img);
		}
	}

	fprintf(stderr, "[baker] Side '%s': %u codes, %u sprite sheets extracted\n",
	        side_name, code_count, sprites_extracted);
	xcr_free(archive);
}

static void bake_sprites(const char* wbc3_root, const char* output_dir)
{
	fprintf(stderr, "\n[baker] === Phase 2: Unit Sprites ===\n");

	// Known side XCR names (skip voice archives)
	static const char* side_xcrs[] = {
		"Barbarians.xcr", "Daemons.xcr", "DarkDwarves.xcr", "DarkElves.xcr",
		"Dragons.xcr", "Dwarves.xcr", "Empire.xcr", "Fey.xcr", "Fliers.xcr",
		"HighElves.xcr", "Knights.xcr", "LairsShops.xcr", "Minotaurs.xcr",
		"Misc.xcr", "Orcs.xcr", "Plaguelords.xcr", "Resources.xcr",
		"Siege.xcr", "Ssrathi.xcr", "Swarm.xcr", "Temples.xcr",
		"TowersWalls.xcr", "Undead.xcr", "WoodElves.xcr",
		NULL
	};

	for (int i = 0; side_xcrs[i]; i++)
		bake_side_sprites(wbc3_root, output_dir, side_xcrs[i]);
}

// ---------------------------------------------------------------------------
// Phase 3: Sound extraction
// ---------------------------------------------------------------------------

static void bake_sounds(const char* wbc3_root, const char* output_dir)
{
	fprintf(stderr, "\n[baker] === Phase 3: Sounds ===\n");

	// SoundFX.xcr
	char sfx_path[512];
	snprintf(sfx_path, sizeof(sfx_path), "%s/Assets/SoundFX.xcr", wbc3_root);
	xcr_archive* sfx = xcr_load(sfx_path);
	if (sfx)
	{
		char sfx_out[512];
		snprintf(sfx_out, sizeof(sfx_out), "%s/sounds/sfx", output_dir);
		mkdirs(sfx_out);

		uint32_t count = 0;
		for (uint32_t i = 0; i < sfx->resource_count; i++)
		{
			const xcr_resource* res = &sfx->resources[i];
			if (res->type != XCR_RESOURCE_WAV) continue;

			char wav_name[256];
			to_lower(wav_name, res->name, sizeof(wav_name));

			char wav_path[512];
			snprintf(wav_path, sizeof(wav_path), "%s/%s", sfx_out, wav_name);
			if (write_file(wav_path, res->data, res->size))
				count++;
		}
		fprintf(stderr, "[baker] SoundFX: %u WAVs extracted\n", count);
		xcr_free(sfx);
	}
	else
	{
		fprintf(stderr, "[baker] SoundFX.xcr not found: %s\n", sfx_path);
	}

	// Per-side voice archives
	static const char* voice_xcrs[] = {
		"BarbariansVoicesEn.xcr", "DaemonsVoicesEn.xcr", "DarkDwarvesVoicesEn.xcr",
		"DarkElvesVoicesEn.xcr", "DefaultVoicesEn.xcr", "DragonsVoicesEn.xcr",
		"DwarvesVoicesEn.xcr", "EmpireVoicesEn.xcr", "FeyVoicesEn.xcr",
		"FliersVoicesEn.xcr", "HighElvesVoicesEn.xcr", "KnightsVoicesEn.xcr",
		"MinotaursVoicesEn.xcr", "MiscVoicesEn.xcr", "OrcsVoicesEn.xcr",
		"PlagueLordsVoicesEn.xcr", "SiegeVoicesEn.xcr", "SsrathiVoicesEn.xcr",
		"SwarmVoicesEn.xcr", "TemplesVoicesEn.xcr", "UndeadVoicesEn.xcr",
		"WoodElvesVoicesEn.xcr",
		NULL
	};

	for (int i = 0; voice_xcrs[i]; i++)
	{
		char vcr_path[512];
		snprintf(vcr_path, sizeof(vcr_path), "%s/Assets/Sides/%s", wbc3_root, voice_xcrs[i]);
		xcr_archive* vcr = xcr_load(vcr_path);
		if (!vcr) continue;

		// Derive side name: "KnightsVoicesEn.xcr" -> "knights"
		char side_name[64];
		strip_ext(side_name, voice_xcrs[i], sizeof(side_name));
		// Remove "VoicesEn" suffix
		char* voices = strstr(side_name, "VoicesEn");
		if (!voices) voices = strstr(side_name, "voicesen");
		if (voices) *voices = '\0';
		to_lower(side_name, side_name, sizeof(side_name));

		char voice_out[512];
		snprintf(voice_out, sizeof(voice_out), "%s/sounds/voices/%s", output_dir, side_name);
		mkdirs(voice_out);

		uint32_t count = 0;
		for (uint32_t j = 0; j < vcr->resource_count; j++)
		{
			const xcr_resource* res = &vcr->resources[j];
			if (res->type != XCR_RESOURCE_WAV) continue;

			char wav_name[256];
			to_lower(wav_name, res->name, sizeof(wav_name));

			char wav_path[512];
			snprintf(wav_path, sizeof(wav_path), "%s/%s", voice_out, wav_name);
			write_file(wav_path, res->data, res->size);
			count++;
		}
		fprintf(stderr, "[baker] Voices '%s': %u WAVs extracted\n", side_name, count);
		xcr_free(vcr);
	}
}

// ---------------------------------------------------------------------------
// Phase 4: Config data (Army.cfg -> units.bin)
// ---------------------------------------------------------------------------

static void bake_configs(const char* wbc3_root, const char* output_dir)
{
	fprintf(stderr, "\n[baker] === Phase 4: Config Data ===\n");

	char data_out[512];
	snprintf(data_out, sizeof(data_out), "%s/data", output_dir);
	mkdirs(data_out);

	// Try System.xcr first (Assets/Data/), then Data.xcr (Data/)
	char data_xcr_path[512];
	snprintf(data_xcr_path, sizeof(data_xcr_path), "%s/Assets/Data/System.xcr", wbc3_root);
	xcr_archive* data_xcr = xcr_load(data_xcr_path);
	if (!data_xcr)
	{
		// Fallback to Data/Data.xcr
		snprintf(data_xcr_path, sizeof(data_xcr_path), "%s/Data/Data.xcr", wbc3_root);
		data_xcr = xcr_load(data_xcr_path);
	}
	if (!data_xcr)
	{
		fprintf(stderr, "[baker] No data XCR found\n");
		return;
	}

	// List all resources for debugging
	fprintf(stderr, "[baker] Data archive resources:\n");
	for (uint32_t i = 0; i < data_xcr->resource_count; i++)
		fprintf(stderr, "  [%u] '%s' (%u bytes, type %d)\n", i,
		        data_xcr->resources[i].name, data_xcr->resources[i].size,
		        data_xcr->resources[i].type);

	// Find Army.cfg resource (by name or by type)
	const xcr_resource* army_cfg = xcr_find(data_xcr, "Army.cfg");
	if (!army_cfg)
	{
		// Try finding any .cfg resource
		for (uint32_t i = 0; i < data_xcr->resource_count; i++)
		{
			if (data_xcr->resources[i].type == XCR_RESOURCE_CFG)
			{
				const char* name = data_xcr->resources[i].name;
				if (strstr(name, "rmy") || strstr(name, "RMY") || strstr(name, "army"))
				{
					army_cfg = &data_xcr->resources[i];
					break;
				}
			}
		}
	}

	if (army_cfg)
	{
		fprintf(stderr, "[baker] Found Army.cfg: %u bytes\n", army_cfg->size);

		// The cfg data is text — ensure null-termination
		char* cfg_text = malloc(army_cfg->size + 1);
		memcpy(cfg_text, army_cfg->data, army_cfg->size);
		cfg_text[army_cfg->size] = '\0';

		const xcr_resource* bldg_cfg = xcr_find(data_xcr, "Building.cfg");

		// Parse army defs from Army.cfg
		baked_unit_def* defs = NULL;
		uint32_t count = cfg_parse_armies(cfg_text, army_cfg->size, &defs);

		// Merge stats from Building.cfg [RULES] section (which covers BOTH buildings and armies)
		if (count > 0 && defs && bldg_cfg)
		{
			char* bldg_text2 = malloc(bldg_cfg->size + 1);
			memcpy(bldg_text2, bldg_cfg->data, bldg_cfg->size);
			bldg_text2[bldg_cfg->size] = '\0';
			cfg_merge_rules(bldg_text2, defs, count);
			free(bldg_text2);
		}

		if (count > 0 && defs)
		{
			char bin_path[512];
			snprintf(bin_path, sizeof(bin_path), "%s/units.bin", data_out);
			cfg_write_units_bin(bin_path, defs, count);

			uint32_t with_hits = 0;
			for (uint32_t i = 0; i < count; i++)
				if (defs[i].hits > 0) with_hits++;
			fprintf(stderr, "[baker] %u/%u units have [RULES] stats\n", with_hits, count);

			// Print a few examples
			for (uint32_t i = 0; i < count && i < 5; i++)
				fprintf(stderr, "  %s '%s': HP=%u DMG=%u ARM=%u SPD=%u COST=%u/%u/%u/%u\n",
				        defs[i].code, defs[i].name, defs[i].hits, defs[i].damage,
				        defs[i].armor, defs[i].speed,
				        defs[i].cost_gold, defs[i].cost_metal,
				        defs[i].cost_crystal, defs[i].cost_stone);
		}
		free(defs);
		free(cfg_text);
	}
	else
	{
		fprintf(stderr, "[baker] Army.cfg not found in Data.xcr\n");

		// List what IS in Data.xcr for debugging
		fprintf(stderr, "[baker] Data.xcr contains %u resources:\n", data_xcr->resource_count);
		fprintf(stderr, "[baker] Data archive: %u resources\n", data_xcr->resource_count);
	}

	xcr_free(data_xcr);
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage: asset_baker <wbc3_install_dir> <output_dir>\n");
		fprintf(stderr, "\nConverts WBC3 game data into baked assets for Open Battlecry.\n");
		return 1;
	}

	const char* wbc3_root = argv[1];
	const char* output_dir = argv[2];

	fprintf(stderr, "[baker] WBC3 root: %s\n", wbc3_root);
	fprintf(stderr, "[baker] Output:    %s\n", output_dir);

	mkdirs(output_dir);

	bake_terrain(wbc3_root, output_dir);
	bake_sprites(wbc3_root, output_dir);
	bake_sounds(wbc3_root, output_dir);
	bake_configs(wbc3_root, output_dir);

	fprintf(stderr, "\n[baker] All phases complete.\n");
	return 0;
}
