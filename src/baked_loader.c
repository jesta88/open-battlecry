#include "baked_loader.h"
#include "gfx.h"
#include "image.h"
#include "ani.h"
#include "audio.h"
#include "file_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
// Load terrain textures from BMP files
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

	uint32_t loaded = 0;
	for (int t = 0; t < TERRAIN_COUNT; t++)
	{
		char bmp_path[512];
		snprintf(bmp_path, sizeof(bmp_path), "%s/%s.bmp", terrain_dir, terrain_names[t]);

		size_t bmp_size = 0;
		uint8_t* bmp_data = read_file_binary(bmp_path, &bmp_size);
		if (bmp_data)
		{
			image img = {0};
			if (image_decode_bmp(bmp_data, (uint32_t)bmp_size, &img))
			{
				textures[t] = gfx_create_texture(img.width, img.height, img.pixels);
				image_free(&img);
				loaded++;
				free(bmp_data);
				continue;
			}
			free(bmp_data);
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
// Load unit type sprites from baked BMP + ANI files
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

	// Load each animation sprite sheet
	for (int i = 0; i < ANI_TYPE_COUNT; i++)
	{
		if (!ut->ani.types[i].used) continue;

		char bmp_path[512];
		snprintf(bmp_path, sizeof(bmp_path), "%s/%s%s.bmp", side_dir, code, anim_suffixes[i]);

		size_t bmp_size = 0;
		uint8_t* bmp_data = read_file_binary(bmp_path, &bmp_size);
		if (!bmp_data) continue;

		image img = {0};
		if (image_decode_bmp(bmp_data, (uint32_t)bmp_size, &img))
		{
			ut->tex[i] = gfx_create_texture(img.width, img.height, img.pixels);
			ut->sheet_w[i] = img.width;
			ut->sheet_h[i] = img.height;
			image_free(&img);
		}
		free(bmp_data);
	}

	return ut->tex[ANI_STAND] != UINT32_MAX;
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
// Scan side directory for unit codes (by .ani files)
// ---------------------------------------------------------------------------

uint32_t baked_scan_side_codes(const char* side_dir, char codes[][8], uint32_t max_codes)
{
	// We can't use directory listing portably in pure C without platform APIs.
	// Instead, try loading every code from the baked unit definitions.
	// This function is a placeholder — the caller should use baked_unit_def codes instead.
	(void)side_dir;
	(void)codes;
	(void)max_codes;
	return 0;
}
