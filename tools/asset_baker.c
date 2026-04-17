#include "xcr.h"
#include "scn.h"
#include "image.h"
#include "ani.h"
#include "file_io.h"
#include "baked_format.h"
#include "cfg_parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <direct.h>
#include <io.h>
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

// Write RGBA pixels as raw .tex file (header + raw RGBA pixels)
static bool write_tex(const char* path, uint32_t w, uint32_t h, const uint8_t* rgba)
{
	FILE* f = fopen(path, "wb");
	if (!f)
	{
		fprintf(stderr, "[baker] Failed to write TEX: %s\n", path);
		return false;
	}

	baked_tex_header hdr = {.magic = BAKED_TEX_MAGIC, .width = w, .height = h, .reserved = 0};
	fwrite(&hdr, sizeof(hdr), 1, f);
	fwrite(rgba, w * h * 4, 1, f);
	fclose(f);
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

		// Extract the first full-size BMP or RLE resource as the terrain tile.
		// Thumbnail variants have an "i" suffix before the extension (e.g. TGX0i.bmp) — skip them.
		bool found = false;
		for (uint32_t i = 0; i < archive->resource_count && !found; i++)
		{
			const xcr_resource* res = &archive->resources[i];

			if (res->type != XCR_RESOURCE_BMP && res->type != XCR_RESOURCE_RLE) continue;

			// Skip thumbnails: name ends with "i.bmp", "i.rle", etc.
			const char* dot = strrchr(res->name, '.');
			bool is_thumb = (dot && dot > res->name && (dot[-1] == 'i' || dot[-1] == 'I'));

			image img = {0};
			if (res->type == XCR_RESOURCE_BMP)
			{
				if (!image_decode_bmp(res->data, res->size, &img)) continue;
			}
			else
			{
				if (!image_decode_rle(res->data, res->size, &img)) continue;
			}

			if (is_thumb) { image_free(&img); continue; }

			char tex_path[512];
			snprintf(tex_path, sizeof(tex_path), "%s/%s.tex", terrain_out, terrain_map[t].output_name);
			if (write_tex(tex_path, img.width, img.height, img.pixels))
			{
				fprintf(stderr, "[baker] Terrain: %s -> %s (%ux%u) <-- selected\n",
				        terrain_map[t].xcr_name, tex_path, img.width, img.height);
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

			char tex_path[512];
			snprintf(tex_path, sizeof(tex_path), "%s/%s%s.tex",
			         side_out, codes[c], anim_suffix_out[a]);
			if (write_tex(tex_path, img.width, img.height, img.pixels))
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
// Phase 5: Feature extraction
// ---------------------------------------------------------------------------

static const char* feature_xcrs[] = {
	"DesertMisc.xcr",   "DesertTrees.xcr",
	"DirtMisc.xcr",     "DirtTrees.xcr",
	"GrassMisc.xcr",    "GrassPlants.xcr",  "GrassTrees.xcr",
	"OtherMisc.xcr",
	"SnowMisc.xcr",     "SnowTrees.xcr",
	NULL
};

// Raw xFEATURE_TYPE_ANSI on disk (MSVC 32-bit):
//   offset 0:  idCode (4)
//   offset 4:  szName[20] (20 chars)
//   offset 24: xShape.bWidth (1), xShape.bHeight (1), xShape.nExclusion[16] (32)
//   offset 58: fCanIWalkOverIt (1), fCanIFlyOverIt (1), fAmbientSound (1), bFree[4] (4)
//   Raw: 65 bytes, MSVC pads to 68 (alignment 4)
#define RAW_FEATURE_TYPE_MIN 65

static void bake_features(const char* wbc3_root, const char* output_dir)
{
	fprintf(stderr, "\n[baker] === Phase 5: Features ===\n");

	char feat_out[512], data_out[512];
	snprintf(feat_out, sizeof(feat_out), "%s/features", output_dir);
	snprintf(data_out, sizeof(data_out), "%s/data", output_dir);
	mkdirs(feat_out);
	mkdirs(data_out);

	// Collect feature definitions and extract sprites
	baked_feature_def* defs = NULL;
	uint32_t def_count = 0, def_cap = 0;
	uint32_t sprites_extracted = 0;

	for (int f = 0; feature_xcrs[f]; f++)
	{
		char xcr_path[512];
		snprintf(xcr_path, sizeof(xcr_path), "%s/Assets/Features/%s", wbc3_root, feature_xcrs[f]);
		xcr_archive* archive = xcr_load(xcr_path);
		if (!archive) continue;

		// Find unique feature codes from ANI resources (same pattern as unit sprites)
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

			// Skip duplicates within this archive
			bool dup = false;
			for (uint32_t j = 0; j < code_count; j++)
				if (strcmp(codes[j], code) == 0) { dup = true; break; }
			if (dup) continue;

			if (code_count < 256) { memcpy(codes[code_count], code, 8); code_count++; }
		}

		for (uint32_t c = 0; c < code_count; c++)
		{
			// Check if we already have this feature (from another archive)
			bool already = false;
			for (uint32_t d = 0; d < def_count; d++)
			{
				char existing[8] = {0};
				memcpy(existing, &defs[d].code, 4);
				if (strcmp(existing, codes[c]) == 0) { already = true; break; }
			}
			if (already) continue;

			// Extract ANI metadata
			char ani_name[64];
			snprintf(ani_name, sizeof(ani_name), "%s.ANI", codes[c]);
			const xcr_resource* ani_res = xcr_find(archive, ani_name);
			if (ani_res)
			{
				char ani_path[512];
				char lower_code[8];
				to_lower(lower_code, codes[c], sizeof(lower_code));
				snprintf(ani_path, sizeof(ani_path), "%s/%s.ani", feat_out, lower_code);
				write_file(ani_path, ani_res->data, ani_res->size);
			}

			// Extract STAND sprite (.RLE)
			char rle_name[64];
			snprintf(rle_name, sizeof(rle_name), "%s.RLE", codes[c]);
			const xcr_resource* rle_res = xcr_find(archive, rle_name);
			if (rle_res)
			{
				image img = {0};
				if (image_decode_rle(rle_res->data, rle_res->size, &img))
				{
					char lower_code[8];
					to_lower(lower_code, codes[c], sizeof(lower_code));
					char tex_path[512];
					snprintf(tex_path, sizeof(tex_path), "%s/%s.tex", feat_out, lower_code);
					if (write_tex(tex_path, img.width, img.height, img.pixels))
						sprites_extracted++;
					image_free(&img);
				}
			}

			// Parse FEA resource for shape/walkability
			char fea_name[64];
			snprintf(fea_name, sizeof(fea_name), "%s.FEA", codes[c]);
			const xcr_resource* fea_res = xcr_find(archive, fea_name);

			// Grow defs array
			if (def_count >= def_cap)
			{
				def_cap = def_cap ? def_cap * 2 : 128;
				defs = realloc(defs, def_cap * sizeof(baked_feature_def));
			}

			baked_feature_def* def = &defs[def_count++];
			memset(def, 0, sizeof(*def));
			memcpy(&def->code, codes[c], 4);
			def->shape_w = 1;
			def->shape_h = 1;
			def->walkable = 0;
			def->flyable = 1;

			if (fea_res && fea_res->size >= RAW_FEATURE_TYPE_MIN)
			{
				const uint8_t* fd = fea_res->data;
				// offset 4: szName[20]
				memcpy(def->name, fd + 4, 20);
				def->name[19] = '\0';
				// offset 24: xShape.bWidth, offset 25: xShape.bHeight
				def->shape_w = fd[24] > 0 ? fd[24] : 1;
				def->shape_h = fd[25] > 0 ? fd[25] : 1;
				// offset 58: fCanIWalkOverIt, offset 59: fCanIFlyOverIt
				def->walkable = fd[58];
				def->flyable  = fd[59];
			}
		}

		xcr_free(archive);
	}

	// Write features.bin catalog
	if (def_count > 0)
	{
		char bin_path[512];
		snprintf(bin_path, sizeof(bin_path), "%s/features.bin", data_out);
		FILE* fp = fopen(bin_path, "wb");
		if (fp)
		{
			baked_header hdr = {
				.magic = BAKED_FEATURES_MAGIC,
				.version = BAKED_VERSION,
				.record_count = def_count,
				.record_size = sizeof(baked_feature_def),
			};
			fwrite(&hdr, sizeof(hdr), 1, fp);
			fwrite(defs, sizeof(baked_feature_def), def_count, fp);
			fclose(fp);
			fprintf(stderr, "[baker] Wrote %u feature defs to %s\n", def_count, bin_path);
		}
	}

	free(defs);
	fprintf(stderr, "[baker] Features: %u types cataloged, %u sprites extracted\n",
	        def_count, sprites_extracted);
}

// ---------------------------------------------------------------------------
// Phase 6: Scenario conversion (.SCN -> .bscn)
// ---------------------------------------------------------------------------

// All terrain XCR archives that contain tile type definitions (TER resources)
static const char* terrain_xcrs[] = {
	"Caverns.xcr",
	"Desert Cliffs.xcr",
	"Desert.xcr",
	"Dirt.xcr",
	"Grass Coast.xcr",
	"Grass.xcr",
	"High Dirt.xcr",
	"High Grass.xcr",
	"Jungle.xcr",
	"Lava.xcr",
	"Marsh.xcr",
	"Mountains.xcr",
	"Rock Cliffs.xcr",
	"Rock Coast.xcr",
	"Rock.xcr",
	"Snow Coast.xcr",
	"Snow.xcr",
	"Water.xcr",
	NULL
};

// Sanitize a scenario name for use as a filename (lowercase, replace spaces with underscores)
static void sanitize_filename(char* dst, const char* src, size_t max)
{
	size_t i = 0;
	for (; src[i] && i < max - 1; i++)
	{
		char c = (char)tolower((unsigned char)src[i]);
		if (c == ' ') c = '_';
		else if (c == '\'' || c == '\"') continue;
		dst[i] = c;
	}
	dst[i] = '\0';
}

static bool write_bscn(const char* path, const scn_scenario* scn,
                        const uint8_t* terrain, const uint8_t* height,
                        const scn_tile_visuals* vis)
{
	FILE* f = fopen(path, "wb");
	if (!f) return false;

	uint32_t num_sides = 0;
	for (int i = 0; i < SCN_MAX_SIDES; i++)
		if (scn->sides[i].used) num_sides++;

	uint32_t cell_count = scn->width * scn->height;

	baked_scenario_header hdr = {0};
	hdr.magic             = BAKED_SCENARIO_MAGIC;
	hdr.version           = BAKED_SCENARIO_VERSION;
	hdr.width             = scn->width;
	hdr.height            = scn->height;
	hdr.num_features      = scn->num_features;
	hdr.num_sides         = num_sides;
	hdr.num_tile_textures = vis->num_tiles;
	memcpy(hdr.name, scn->name, sizeof(hdr.name));
	memcpy(hdr.code, scn->code, sizeof(hdr.code));

	fwrite(&hdr, sizeof(hdr), 1, f);

	// Terrain grid
	fwrite(terrain, 1, cell_count, f);

	// Height grid
	fwrite(height, 1, cell_count, f);

	// Tile visual grid (per-cell)
	for (uint32_t i = 0; i < cell_count; i++)
	{
		baked_cell_visual cv;
		cv.tile_tex_index = vis->cell_tile_index[i];
		cv.local_x        = vis->cell_local_x[i];
		cv.local_y        = vis->cell_local_y[i];
		fwrite(&cv, sizeof(cv), 1, f);
	}

	// Tile texture catalog
	for (uint32_t i = 0; i < vis->num_tiles; i++)
	{
		baked_tile_entry te = {0};
		te.tile_id = vis->tile_ids[i];
		te.cell_w  = vis->tile_cell_w[i];
		te.cell_h  = vis->tile_cell_h[i];
		fwrite(&te, sizeof(te), 1, f);
	}

	// Features
	for (uint32_t i = 0; i < scn->num_features; i++)
	{
		const scn_feature* feat = &scn->features[i];
		baked_scenario_feature bf = {0};
		bf.code      = feat->code;
		bf.x         = (uint16_t)(feat->x >= 0 ? feat->x : 0);
		bf.y         = (uint16_t)(feat->y >= 0 ? feat->y : 0);
		bf.direction = (uint8_t)(feat->direction & 0x7);
		bf.flags     = 0;
		fwrite(&bf, sizeof(bf), 1, f);
	}

	// Sides (only used sides)
	for (int i = 0; i < SCN_MAX_SIDES; i++)
	{
		if (!scn->sides[i].used) continue;
		const scn_side* s = &scn->sides[i];
		baked_scenario_side bs = {0};
		bs.race     = s->race;
		bs.color    = s->color;
		bs.team     = s->team;
		bs.start_x  = s->start_random ? -1 : (int16_t)s->start_x;
		bs.start_y  = s->start_random ? -1 : (int16_t)s->start_y;
		bs.ai_level = (uint8_t)s->ai_level;
		bs.flags    = (s->must_be_human ? 0x01 : 0) | (s->must_be_on ? 0x02 : 0);
		fwrite(&bs, sizeof(bs), 1, f);
	}

	fclose(f);
	return true;
}

// Extract all non-thumbnail tile BMPs from terrain XCRs to baked/terrain/tiles/
static void extract_tile_bmps(const char* wbc3_root, const char* output_dir)
{
	char tiles_dir[512];
	snprintf(tiles_dir, sizeof(tiles_dir), "%s/terrain/tiles", output_dir);
	mkdirs(tiles_dir);

	uint32_t total = 0;
	for (int t = 0; terrain_xcrs[t]; t++)
	{
		char xcr_path[512];
		snprintf(xcr_path, sizeof(xcr_path), "%s/Assets/Terrain/%s", wbc3_root, terrain_xcrs[t]);
		xcr_archive* archive = xcr_load(xcr_path);
		if (!archive) continue;

		for (uint32_t i = 0; i < archive->resource_count; i++)
		{
			const xcr_resource* res = &archive->resources[i];
			if (res->type != XCR_RESOURCE_BMP) continue;

			// Skip thumbnails (name ends with 'i' before extension)
			const char* dot = strrchr(res->name, '.');
			if (dot && dot > res->name && (dot[-1] == 'i' || dot[-1] == 'I')) continue;

			image img = {0};
			if (!image_decode_bmp(res->data, res->size, &img)) continue;

			// Output filename: lowercase name without extension
			char base[64];
			strip_ext(base, res->name, sizeof(base));
			to_lower(base, base, sizeof(base));

			char tex_path[512];
			snprintf(tex_path, sizeof(tex_path), "%s/%s.tex", tiles_dir, base);
			if (write_tex(tex_path, img.width, img.height, img.pixels))
				total++;
			image_free(&img);
		}

		xcr_free(archive);
	}

	fprintf(stderr, "[baker] Extracted %u tile textures to %s\n", total, tiles_dir);
}

static void bake_scenarios(const char* wbc3_root, const char* output_dir)
{
	fprintf(stderr, "\n[baker] === Phase 6: Scenarios ===\n");

	// Step 1: Build tile type catalog from all terrain XCR archives
	scn_tile_catalog catalog;
	scn_tile_catalog_init(&catalog);

	for (int i = 0; terrain_xcrs[i]; i++)
	{
		char xcr_path[512];
		snprintf(xcr_path, sizeof(xcr_path), "%s/Assets/Terrain/%s", wbc3_root, terrain_xcrs[i]);
		if (scn_load_tile_types(&catalog, xcr_path))
			fprintf(stderr, "[baker] Loaded tile types from %s\n", terrain_xcrs[i]);
	}

	fprintf(stderr, "[baker] Tile catalog: %u tile types loaded\n", catalog.count);
	if (catalog.count == 0)
	{
		fprintf(stderr, "[baker] No tile types found — skipping scenario baking\n");
		scn_tile_catalog_free(&catalog);
		return;
	}

	// Step 1b: Extract all tile BMP textures
	extract_tile_bmps(wbc3_root, output_dir);

	// Step 2: Scan Scenario/*.SCN
	char scn_out[512];
	snprintf(scn_out, sizeof(scn_out), "%s/scenarios", output_dir);
	mkdirs(scn_out);

	char pattern[512];
	snprintf(pattern, sizeof(pattern), "%s/Scenario/*.SCN", wbc3_root);

	uint32_t converted = 0, failed = 0;

	struct _finddata_t fd;
	intptr_t hfind = _findfirst(pattern, &fd);
	if (hfind == -1)
	{
		fprintf(stderr, "[baker] No scenario files found in %s/Scenario/\n", wbc3_root);
		scn_tile_catalog_free(&catalog);
		return;
	}

	do
	{
		char scn_path[512];
		snprintf(scn_path, sizeof(scn_path), "%s/Scenario/%s", wbc3_root, fd.name);

		scn_scenario scn;
		if (!scn_load_scenario(scn_path, &scn))
		{
			fprintf(stderr, "[baker] Failed to parse: %s\n", fd.name);
			failed++;
			continue;
		}

		fprintf(stderr, "[baker] Scenario '%s' (%s): %ux%u cells, %u features, %u players\n",
		        scn.name, scn.code, scn.width, scn.height, scn.num_features, scn.num_players);

		// Step 3: Expand terrain
		uint32_t cell_count = scn.width * scn.height;
		uint8_t* terrain = calloc(cell_count, 1);
		uint8_t* height  = calloc(cell_count, 1);

		if (scn_expand_terrain(&scn, &catalog, terrain, height))
		{
			// Step 3b: Build per-cell tile visual references
			scn_tile_visuals vis;
			scn_build_tile_visuals(&scn, &catalog, &vis);

			// Step 4: Write .bscn
			char safe_name[64];
			if (scn.code[0])
				sanitize_filename(safe_name, scn.code, sizeof(safe_name));
			else
				sanitize_filename(safe_name, fd.name, sizeof(safe_name));

			// Strip .scn extension if present in safe_name
			char* ext = strstr(safe_name, ".scn");
			if (ext) *ext = '\0';

			char bscn_path[512];
			snprintf(bscn_path, sizeof(bscn_path), "%s/%s.bscn", scn_out, safe_name);

			if (write_bscn(bscn_path, &scn, terrain, height, &vis))
			{
				fprintf(stderr, "[baker] -> %s\n", bscn_path);
				converted++;
			}
			else
			{
				fprintf(stderr, "[baker] Failed to write: %s\n", bscn_path);
				failed++;
			}

			scn_free_tile_visuals(&vis);
		}
		else
		{
			fprintf(stderr, "[baker] No terrain resolved for: %s\n", fd.name);
			failed++;
		}

		free(terrain);
		free(height);
		scn_free_scenario(&scn);
	} while (_findnext(hfind, &fd) == 0);

	_findclose(hfind);
	scn_tile_catalog_free(&catalog);

	fprintf(stderr, "[baker] Scenarios: %u converted, %u failed\n", converted, failed);
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
	bake_features(wbc3_root, output_dir);
	bake_scenarios(wbc3_root, output_dir);

	fprintf(stderr, "\n[baker] All phases complete.\n");
	return 0;
}
