#include "baked_loader.h"
#include "baked_format.h"
#include "entity.h"
#include "map.h"
#include "gfx.h"
#include "image.h"
#include "ani.h"
#include "audio.h"
#include "file_io.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>

// ---------------------------------------------------------------------------
// Multithreaded batch image loader
// ---------------------------------------------------------------------------

typedef struct
{
	char path[512];     // primary path (.tex)
	char fallback[512]; // fallback path (.bmp)
	image img;
	bool loaded;
} load_item;

typedef struct
{
	load_item* items;
	uint32_t count;
	volatile LONG next; // atomic work index
} load_batch;

static unsigned __stdcall load_worker(void* param)
{
	load_batch* batch = (load_batch*)param;
	for (;;)
	{
		uint32_t i = (uint32_t)InterlockedExchangeAdd(&batch->next, 1);
		if (i >= batch->count) break;

		load_item* item = &batch->items[i];
		size_t size = 0;

		// Try primary path (.tex) first
		uint8_t* data = read_file_binary(item->path, &size);
		if (!data && item->fallback[0])
			data = read_file_binary(item->fallback, &size);
		if (!data) continue;

		// Try tex format first (checks magic), then BMP
		item->loaded = image_decode_tex(data, (uint32_t)size, &item->img)
		            || image_decode_bmp(data, (uint32_t)size, &item->img);
		free(data);
	}
	return 0;
}

static uint32_t get_num_cores(void)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

static void batch_load_parallel(load_item* items, uint32_t count)
{
	if (count == 0) return;

	uint32_t num_threads = get_num_cores();
	if (num_threads > 32) num_threads = 32;
	if (num_threads > count) num_threads = count;

	load_batch batch = {.items = items, .count = count, .next = 0};

	HANDLE threads[32];
	for (uint32_t t = 0; t < num_threads; t++)
		threads[t] = (HANDLE)_beginthreadex(NULL, 0, load_worker, &batch, 0, NULL);

	WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);

	for (uint32_t t = 0; t < num_threads; t++)
		CloseHandle(threads[t]);
}

// ---------------------------------------------------------------------------
// Load unit definitions from units.bin
// ---------------------------------------------------------------------------

bool baked_load_units(const char* bin_path, baked_unit_def** out, uint32_t* count)
{
	size_t file_size = 0;
	uint8_t* data = read_file_binary(bin_path, &file_size);
	if (!data)
	{
		fprintf(stderr, "[baked] Failed to read: %s\n", bin_path);
		return false;
	}

	if (file_size < sizeof(baked_header))
	{
		free(data);
		return false;
	}

	baked_header* hdr = (baked_header*)data;
	if (hdr->magic != BAKED_UNITS_MAGIC || hdr->version != BAKED_VERSION)
	{
		fprintf(stderr, "[baked] Invalid units.bin: magic=0x%08X version=%u\n", hdr->magic, hdr->version);
		free(data);
		return false;
	}

	if (hdr->record_size != sizeof(baked_unit_def))
	{
		fprintf(stderr, "[baked] units.bin record size mismatch: file=%u expected=%zu\n",
		        hdr->record_size, sizeof(baked_unit_def));
		free(data);
		return false;
	}

	size_t expected = sizeof(baked_header) + (size_t)hdr->record_count * hdr->record_size;
	if (file_size < expected)
	{
		free(data);
		return false;
	}

	*count = hdr->record_count;
	*out = malloc((size_t)hdr->record_count * sizeof(baked_unit_def));
	memcpy(*out, data + sizeof(baked_header), (size_t)hdr->record_count * sizeof(baked_unit_def));
	free(data);

	fprintf(stderr, "[baked] Loaded %u unit definitions from %s\n", *count, bin_path);
	return true;
}

// ---------------------------------------------------------------------------
// Load terrain textures (multithreaded)
// ---------------------------------------------------------------------------

bool baked_load_terrain(const char* terrain_dir, uint32_t textures[])
{
	static const char* terrain_names[] = {
		"grass", "dirt", "sand", "water", "rock",
		"lava", "marsh", "ford", "snow", "void",
		"mountain", "walls", "impassable",
	};

	// Procedural fallback colors for missing terrain
	static const uint8_t fallback_colors[][3] = {
		{ 50, 120,  40}, {130,  90,  50}, {200, 180, 100}, { 30,  60, 160}, {128, 128, 128},
		{200,  60,  20}, { 40,  80,  50}, { 80, 140, 180}, {220, 225, 230}, { 10,  10,  10},
		{ 80,  75,  70}, {160, 140, 110}, {140,  30,  30},
	};

	// Build batch: try .tex first, fall back to .bmp
	load_item items[TERRAIN_COUNT];
	memset(items, 0, sizeof(items));

	for (int t = 0; t < TERRAIN_COUNT; t++)
	{
		snprintf(items[t].path, sizeof(items[t].path), "%s/%s.tex", terrain_dir, terrain_names[t]);
		snprintf(items[t].fallback, sizeof(items[t].fallback), "%s/%s.bmp", terrain_dir, terrain_names[t]);
	}

	batch_load_parallel(items, TERRAIN_COUNT);

	uint32_t loaded = 0;
	for (int t = 0; t < TERRAIN_COUNT; t++)
	{
		if (items[t].loaded)
		{
			textures[t] = gfx_create_texture(items[t].img.width, items[t].img.height, items[t].img.pixels);
			image_free(&items[t].img);
			loaded++;
			continue;
		}

		// Fallback: procedural 8x8 texture
		uint8_t pixels[8 * 8 * 4];
		for (int py = 0; py < 8; py++)
			for (int px = 0; px < 8; px++)
			{
				int idx = (py * 8 + px) * 4;
				int noise = ((px * 7 + py * 13) % 11) - 5;
				int r = (int)fallback_colors[t][0] + noise;
				int g = (int)fallback_colors[t][1] + noise;
				int b = (int)fallback_colors[t][2] + noise;
				pixels[idx + 0] = (uint8_t)(r < 0 ? 0 : (r > 255 ? 255 : r));
				pixels[idx + 1] = (uint8_t)(g < 0 ? 0 : (g > 255 ? 255 : g));
				pixels[idx + 2] = (uint8_t)(b < 0 ? 0 : (b > 255 ? 255 : b));
				pixels[idx + 3] = 255;
			}
		textures[t] = gfx_create_texture(8, 8, pixels);
	}

	fprintf(stderr, "[baked] Loaded %u/%d terrain textures from %s\n", loaded, TERRAIN_COUNT, terrain_dir);
	return loaded > 0;
}

// ---------------------------------------------------------------------------
// Load single unit type sprites (multithreaded internally)
// ---------------------------------------------------------------------------

static const char* anim_suffixes[] = {"", "W", "F", "D", "A", "Z", "C", "S", "I"};

bool baked_load_unit_type(unit_type* ut, const char* side_dir, const char* code)
{
	memset(ut, 0, sizeof(*ut));
	for (int i = 0; i < ANI_TYPE_COUNT; i++)
		ut->tex[i] = UINT32_MAX;
	snprintf(ut->base_code, sizeof(ut->base_code), "%s", code);

	// Load ANI metadata
	char ani_path[512];
	snprintf(ani_path, sizeof(ani_path), "%s/%s.ani", side_dir, code);
	size_t ani_size = 0;
	uint8_t* ani_data = read_file_binary(ani_path, &ani_size);
	if (!ani_data || !ani_parse(ani_data, (uint32_t)ani_size, &ut->ani))
	{
		free(ani_data);
		return false;
	}
	free(ani_data);

	// Batch load all animation sprite sheets in parallel
	load_item items[ANI_TYPE_COUNT];
	memset(items, 0, sizeof(items));
	uint32_t batch_count = 0;
	int batch_to_ani[ANI_TYPE_COUNT];

	for (int i = 0; i < ANI_TYPE_COUNT; i++)
	{
		if (!ut->ani.types[i].used) continue;
		snprintf(items[batch_count].path, sizeof(items[batch_count].path),
		         "%s/%s%s.tex", side_dir, code, anim_suffixes[i]);
		snprintf(items[batch_count].fallback, sizeof(items[batch_count].fallback),
		         "%s/%s%s.bmp", side_dir, code, anim_suffixes[i]);
		batch_to_ani[batch_count] = i;
		batch_count++;
	}

	batch_load_parallel(items, batch_count);

	// Upload to GPU (main thread)
	for (uint32_t b = 0; b < batch_count; b++)
	{
		if (!items[b].loaded) continue;
		int i = batch_to_ani[b];
		ut->tex[i] = gfx_create_texture(items[b].img.width, items[b].img.height, items[b].img.pixels);
		ut->sheet_w[i] = items[b].img.width;
		ut->sheet_h[i] = items[b].img.height;
		image_free(&items[b].img);
	}

	return ut->tex[ANI_STAND] != UINT32_MAX;
}

// ---------------------------------------------------------------------------
// Batch-load all unit types from a side (maximum parallelism)
// ---------------------------------------------------------------------------

uint32_t baked_load_side_units(unit_type* out_types, uint32_t max_types,
                               const char* side_dir,
                               const baked_unit_def* defs, uint32_t num_defs)
{
	// Phase 1: Scan ANI files to find valid units and count total images needed
	typedef struct
	{
		uint32_t def_idx;
		ani_info ani;
		uint32_t first_item; // index into items array
		uint32_t item_count;
	} unit_load_info;

	uint32_t max_items = num_defs * ANI_TYPE_COUNT;
	unit_load_info* infos = calloc(num_defs, sizeof(unit_load_info));
	load_item* items = calloc(max_items, sizeof(load_item));
	int* item_ani_idx = calloc(max_items, sizeof(int));
	if (!infos || !items || !item_ani_idx)
	{
		free(infos); free(items); free(item_ani_idx);
		return 0;
	}

	uint32_t num_units = 0;
	uint32_t total_items = 0;

	for (uint32_t d = 0; d < num_defs && num_units < max_types; d++)
	{
		// Load ANI metadata
		char ani_path[512];
		snprintf(ani_path, sizeof(ani_path), "%s/%s.ani", side_dir, defs[d].code);
		size_t ani_size = 0;
		uint8_t* ani_data = read_file_binary(ani_path, &ani_size);
		if (!ani_data) continue;

		ani_info ani = {0};
		if (!ani_parse(ani_data, (uint32_t)ani_size, &ani))
		{
			free(ani_data);
			continue;
		}
		free(ani_data);

		infos[num_units].def_idx = d;
		infos[num_units].ani = ani;
		infos[num_units].first_item = total_items;

		uint32_t unit_items = 0;
		for (int a = 0; a < ANI_TYPE_COUNT; a++)
		{
			if (!ani.types[a].used) continue;
			snprintf(items[total_items].path, sizeof(items[total_items].path),
			         "%s/%s%s.tex", side_dir, defs[d].code, anim_suffixes[a]);
			snprintf(items[total_items].fallback, sizeof(items[total_items].fallback),
			         "%s/%s%s.bmp", side_dir, defs[d].code, anim_suffixes[a]);
			item_ani_idx[total_items] = a;
			total_items++;
			unit_items++;
		}
		infos[num_units].item_count = unit_items;
		num_units++;
	}

	// Phase 2: Decode all sprite sheets in parallel
	fprintf(stderr, "[baked] Batch loading %u images for %u units from %s...\n",
	        total_items, num_units, side_dir);
	batch_load_parallel(items, total_items);

	// Phase 3: Upload to GPU (main thread) and assemble unit_types
	uint32_t loaded = 0;
	for (uint32_t u = 0; u < num_units && loaded < max_types; u++)
	{
		unit_type* ut = &out_types[loaded];
		memset(ut, 0, sizeof(*ut));
		for (int i = 0; i < ANI_TYPE_COUNT; i++)
			ut->tex[i] = UINT32_MAX;

		const baked_unit_def* def = &defs[infos[u].def_idx];
		snprintf(ut->base_code, sizeof(ut->base_code), "%s", def->code);
		ut->ani = infos[u].ani;

		for (uint32_t b = 0; b < infos[u].item_count; b++)
		{
			uint32_t item_idx = infos[u].first_item + b;
			if (!items[item_idx].loaded) continue;

			int a = item_ani_idx[item_idx];
			ut->tex[a] = gfx_create_texture(items[item_idx].img.width, items[item_idx].img.height,
			                                items[item_idx].img.pixels);
			ut->sheet_w[a] = items[item_idx].img.width;
			ut->sheet_h[a] = items[item_idx].img.height;
			image_free(&items[item_idx].img);
		}

		// Only count units that have at least stand + walk animations
		if (ut->tex[ANI_STAND] != UINT32_MAX && ut->tex[ANI_WALK] != UINT32_MAX)
			loaded++;
	}

	free(infos);
	free(items);
	free(item_ani_idx);

	fprintf(stderr, "[baked] Loaded %u unit types from %s\n", loaded, side_dir);
	return loaded;
}

// ---------------------------------------------------------------------------
// Load sound from baked WAV file
// ---------------------------------------------------------------------------

uint32_t baked_load_sound(const char* wav_path)
{
	size_t wav_size = 0;
	uint8_t* wav_data = read_file_binary(wav_path, &wav_size);
	if (!wav_data) return UINT32_MAX;

	uint32_t handle = audio_load_wav(wav_data, (uint32_t)wav_size);
	free(wav_data);
	return handle;
}

// ---------------------------------------------------------------------------
// Load baked scenario (.bscn)
// ---------------------------------------------------------------------------

bool baked_load_scenario(const char* bscn_path, const char* tiles_dir,
                         const char* features_dir, game_map* map,
                         scenario_visuals* vis, baked_scenario_header* hdr_out)
{
	size_t file_size = 0;
	uint8_t* data = read_file_binary(bscn_path, &file_size);
	if (!data)
	{
		fprintf(stderr, "[baked] Failed to read: %s\n", bscn_path);
		return false;
	}

	if (file_size < sizeof(baked_scenario_header))
	{
		fprintf(stderr, "[baked] Scenario file too small: %s\n", bscn_path);
		free(data);
		return false;
	}

	const baked_scenario_header* hdr = (const baked_scenario_header*)data;
	if (hdr->magic != BAKED_SCENARIO_MAGIC)
	{
		fprintf(stderr, "[baked] Invalid scenario magic: 0x%08X in %s\n", hdr->magic, bscn_path);
		free(data);
		return false;
	}

	uint32_t w = hdr->width;
	uint32_t h = hdr->height;
	size_t cell_count = (size_t)w * h;

	// Compute expected file layout based on version
	bool has_tile_visuals = (hdr->version >= 2);
	bool has_full_features = (hdr->version >= 3);
	uint32_t num_tile_tex = has_tile_visuals ? hdr->num_tile_textures : 0;
	uint32_t num_features = hdr->num_features;

	size_t offset = sizeof(baked_scenario_header);
	size_t terrain_off = offset;                           offset += cell_count;
	size_t height_off  = offset;                           offset += cell_count;
	size_t visuals_off = offset;  if (has_tile_visuals)    offset += cell_count * sizeof(baked_cell_visual);
	size_t catalog_off = offset;  if (has_tile_visuals)    offset += num_tile_tex * sizeof(baked_tile_entry);
	size_t features_off = offset;                          offset += num_features * sizeof(baked_scenario_feature);

	if (file_size < offset)
	{
		fprintf(stderr, "[baked] Scenario file truncated: %s\n", bscn_path);
		free(data);
		return false;
	}

	const uint8_t* terrain_data = data + terrain_off;
	const uint8_t* height_data  = data + height_off;

	// Initialize map and fill cells
	if (!map_init(map, w, h))
	{
		free(data);
		return false;
	}

	for (uint32_t y = 0; y < h; y++)
	{
		for (uint32_t x = 0; x < w; x++)
		{
			size_t idx = (size_t)y * w + x;
			map_cell cell = {
				.terrain = terrain_data[idx],
				.height  = height_data[idx],
				.flags   = 0,
			};
			map_set(map, x, y, cell);
		}
	}

	// Load tile visuals if present and requested
	if (vis && has_tile_visuals && tiles_dir)
	{
		memset(vis, 0, sizeof(*vis));
		vis->width  = w;
		vis->height = h;
		vis->num_tile_textures = num_tile_tex;

		// Copy per-cell visual data
		vis->cell_tile_index = malloc(cell_count * sizeof(uint16_t));
		vis->cell_local_x    = malloc(cell_count);
		vis->cell_local_y    = malloc(cell_count);

		const baked_cell_visual* cv = (const baked_cell_visual*)(data + visuals_off);
		for (size_t i = 0; i < cell_count; i++)
		{
			vis->cell_tile_index[i] = cv[i].tile_tex_index;
			vis->cell_local_x[i]    = cv[i].local_x;
			vis->cell_local_y[i]    = cv[i].local_y;
		}

		// Read catalog and load tile textures
		const baked_tile_entry* catalog = (const baked_tile_entry*)(data + catalog_off);
		vis->tile_textures = malloc(num_tile_tex * sizeof(uint32_t));
		vis->tile_cell_w   = malloc(num_tile_tex);
		vis->tile_cell_h   = malloc(num_tile_tex);

		// Build batch load items for parallel texture loading
		load_item* items = calloc(num_tile_tex, sizeof(load_item));
		for (uint32_t i = 0; i < num_tile_tex; i++)
		{
			vis->tile_cell_w[i] = catalog[i].cell_w;
			vis->tile_cell_h[i] = catalog[i].cell_h;
			vis->tile_textures[i] = UINT32_MAX;

			// Convert tile_id (4 bytes) to lowercase filename
			char id_str[8];
			memcpy(id_str, &catalog[i].tile_id, 4);
			id_str[4] = '\0';
			for (int c = 0; c < 4 && id_str[c]; c++)
				id_str[c] = (char)tolower((unsigned char)id_str[c]);

			snprintf(items[i].path, sizeof(items[i].path), "%s/%s.tex", tiles_dir, id_str);
		}

		batch_load_parallel(items, num_tile_tex);

		uint32_t loaded = 0;
		for (uint32_t i = 0; i < num_tile_tex; i++)
		{
			if (items[i].loaded)
			{
				vis->tile_textures[i] = gfx_create_texture(items[i].img.width, items[i].img.height,
				                                            items[i].img.pixels);
				image_free(&items[i].img);
				loaded++;
			}
		}
		free(items);

		fprintf(stderr, "[baked] Loaded %u/%u tile textures\n", loaded, num_tile_tex);
	}

	// Load features if present and requested
	if (vis && has_full_features && features_dir && num_features > 0 &&
	    file_size >= features_off + num_features * sizeof(baked_scenario_feature))
	{
		const baked_scenario_feature* bf = (const baked_scenario_feature*)(data + features_off);

		vis->features = calloc(num_features, sizeof(loaded_feature));
		vis->num_features = num_features;

		// Collect unique feature codes to batch-load textures
		uint32_t unique_codes[1024];
		uint32_t unique_count = 0;

		for (uint32_t i = 0; i < num_features; i++)
		{
			vis->features[i].code      = bf[i].code;
			vis->features[i].x         = bf[i].x;
			vis->features[i].y         = bf[i].y;
			vis->features[i].direction = bf[i].direction;
			vis->features[i].texture   = UINT32_MAX;
			vis->features[i].shape_w   = 1;
			vis->features[i].shape_h   = 1;

			// Track unique codes
			bool found = false;
			for (uint32_t u = 0; u < unique_count; u++)
				if (unique_codes[u] == bf[i].code) { found = true; break; }
			if (!found && unique_count < 1024)
				unique_codes[unique_count++] = bf[i].code;
		}

		// Batch-load textures and ANI for unique feature codes
		load_item* tex_items = calloc(unique_count, sizeof(load_item));
		uint8_t** ani_datas  = calloc(unique_count, sizeof(uint8_t*));
		size_t* ani_sizes    = calloc(unique_count, sizeof(size_t));

		for (uint32_t u = 0; u < unique_count; u++)
		{
			char id_str[8];
			memcpy(id_str, &unique_codes[u], 4);
			id_str[4] = '\0';
			for (int c = 0; c < 4 && id_str[c]; c++)
				id_str[c] = (char)tolower((unsigned char)id_str[c]);

			snprintf(tex_items[u].path, sizeof(tex_items[u].path), "%s/%s.tex", features_dir, id_str);

			// Load ANI file
			char ani_path[512];
			snprintf(ani_path, sizeof(ani_path), "%s/%s.ani", features_dir, id_str);
			ani_datas[u] = read_file_binary(ani_path, &ani_sizes[u]);
		}

		batch_load_parallel(tex_items, unique_count);

		// Upload textures and parse ANI data
		uint32_t feat_tex_handles[1024];
		uint16_t feat_sheet_w[1024], feat_sheet_h[1024];
		ani_info feat_ani[1024];

		for (uint32_t u = 0; u < unique_count; u++)
		{
			feat_tex_handles[u] = UINT32_MAX;
			feat_sheet_w[u] = 0;
			feat_sheet_h[u] = 0;
			memset(&feat_ani[u], 0, sizeof(ani_info));

			if (tex_items[u].loaded)
			{
				feat_tex_handles[u] = gfx_create_texture(tex_items[u].img.width, tex_items[u].img.height,
				                                          tex_items[u].img.pixels);
				feat_sheet_w[u] = (uint16_t)tex_items[u].img.width;
				feat_sheet_h[u] = (uint16_t)tex_items[u].img.height;
				image_free(&tex_items[u].img);
			}

			if (ani_datas[u])
			{
				ani_parse(ani_datas[u], (uint32_t)ani_sizes[u], &feat_ani[u]);
				free(ani_datas[u]);
			}
		}
		free(tex_items);
		free(ani_datas);
		free(ani_sizes);

		// Assign loaded data to each feature instance
		uint32_t features_with_tex = 0;
		for (uint32_t i = 0; i < num_features; i++)
		{
			loaded_feature* lf = &vis->features[i];
			for (uint32_t u = 0; u < unique_count; u++)
			{
				if (unique_codes[u] == lf->code)
				{
					lf->texture  = feat_tex_handles[u];
					lf->sheet_w  = feat_sheet_w[u];
					lf->sheet_h  = feat_sheet_h[u];

					// Use ANI STAND data for origin/frame info
					const ani_type* stand = &feat_ani[u].types[0]; // ANI_STAND = 0
					if (stand->used)
					{
						lf->origin_x   = stand->origin_x;
						lf->origin_y   = stand->origin_y;
						lf->num_frames = stand->num_frames;
						lf->frame_w    = stand->width;
						lf->frame_h    = stand->height;
					}
					else if (lf->sheet_w > 0)
					{
						// Fallback: assume 1 frame, 8 directions
						lf->frame_w    = lf->sheet_w;
						lf->frame_h    = lf->sheet_h / 8;
						lf->num_frames = 1;
					}

					if (lf->texture != UINT32_MAX) features_with_tex++;
					break;
				}
			}
		}

		fprintf(stderr, "[baked] Loaded %u features (%u unique codes, %u with textures)\n",
		        num_features, unique_count, features_with_tex);
	}

	if (hdr_out)
		*hdr_out = *hdr;

	fprintf(stderr, "[baked] Loaded scenario '%s' (%ux%u) from %s\n",
	        hdr->name, w, h, bscn_path);

	free(data);
	return true;
}

void scenario_visuals_free(scenario_visuals* vis)
{
	free(vis->tile_textures);
	free(vis->cell_tile_index);
	free(vis->cell_local_x);
	free(vis->cell_local_y);
	free(vis->tile_cell_w);
	free(vis->tile_cell_h);
	free(vis->features);
	memset(vis, 0, sizeof(*vis));
}
