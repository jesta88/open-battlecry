#include "cfg_parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ---------------------------------------------------------------------------
// Army.cfg column indices (from WBC3 ArmyConfig.cpp)
// ---------------------------------------------------------------------------

#define CODE_INDEX              4
#define STATUSFLAG_INDEX        9
#define CACHE_INDEX             18
#define MISSILE_INDEX           35
#define LATENCY_INDEX           42
#define AISPECIALABILITY_INDEX  50
#define MANA_INDEX              52
#define GENERAL_INDEX           60
#define ARMYNAME_INDEX          62

// [RULES] column indices
#define RULES_CODE_INDEX   0
#define RULES_RACE_INDEX   5
#define RULES_GOLD_INDEX   7
#define RULES_METAL_INDEX  12
#define RULES_STONE_INDEX  17
#define RULES_CRYST_INDEX  22
#define RULES_TIME_INDEX   27
#define RULES_COMBAT_INDEX 32
#define RULES_HITS_INDEX   36
#define RULES_SPEED_INDEX  40
#define RULES_RES_INDEX    44
#define RULES_DMG_INDEX    54
#define RULES_RANGE_INDEX  58

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Read an integer at a fixed column position. Returns 0 if '-' or whitespace.
static int read_int_at(const char* line, int col, int width, int line_len)
{
	if (col >= line_len) return 0;
	if (line[col] == '-' || line[col] == ' ') return 0;

	char buf[16] = {0};
	int n = 0;
	for (int i = col; i < col + width && i < line_len && n < 15; i++)
		buf[n++] = line[i];
	buf[n] = '\0';
	return atoi(buf);
}

// Extract a fixed-width string field, trimming trailing spaces.
static void read_str_at(const char* line, int col, int max_width, int line_len,
                        char* out, int out_size)
{
	int n = 0;
	for (int i = col; i < col + max_width && i < line_len && n < out_size - 1; i++)
		out[n++] = line[i];
	out[n] = '\0';
	// Trim trailing spaces
	while (n > 0 && out[n - 1] == ' ')
		out[--n] = '\0';
}

// Read the rest of the line from a given column.
static void read_rest(const char* line, int col, int line_len, char* out, int out_size)
{
	int n = 0;
	for (int i = col; i < line_len && n < out_size - 1; i++)
	{
		if (line[i] == '\r' || line[i] == '\n') break;
		out[n++] = line[i];
	}
	out[n] = '\0';
	while (n > 0 && out[n - 1] == ' ')
		out[--n] = '\0';
}

// Find section in text. Returns pointer to first line after header, or NULL.
static const char* find_section(const char* text, const char* section)
{
	const char* p = text;
	size_t slen = strlen(section);
	while (*p)
	{
		if (*p == '[' && strncmp(p, section, slen) == 0)
		{
			// Skip to next line
			while (*p && *p != '\n') p++;
			if (*p == '\n') p++;
			return p;
		}
		while (*p && *p != '\n') p++;
		if (*p == '\n') p++;
	}
	return NULL;
}

// Get next non-empty, non-comment line. Returns line length, 0 if end/new section.
static int get_next_line(const char** cursor, char* buf, int buf_size)
{
	while (**cursor)
	{
		const char* start = *cursor;

		// Find end of line
		int len = 0;
		while (start[len] && start[len] != '\n') len++;

		// Advance cursor past this line
		*cursor = start + len;
		if (**cursor == '\n') (*cursor)++;

		// Strip \r
		while (len > 0 && start[len - 1] == '\r') len--;

		// Skip empty lines
		if (len == 0) continue;

		// New section starts = stop
		if (start[0] == '[') return 0;

		// Skip comment lines (starting with '/')
		if (start[0] == '/') continue;

		// Copy to buffer
		if (len >= buf_size) len = buf_size - 1;
		memcpy(buf, start, len);
		buf[len] = '\0';
		return len;
	}
	return 0;
}

// Count lines in a section
static int count_section_lines(const char* text, const char* section)
{
	const char* cursor = find_section(text, section);
	if (!cursor) return 0;
	char buf[512];
	int count = 0;
	while (get_next_line(&cursor, buf, sizeof(buf)) > 0)
		count++;
	return count;
}

// ---------------------------------------------------------------------------
// Movement type from status flag character
// ---------------------------------------------------------------------------

static uint8_t parse_movement(char c)
{
	switch (c)
	{
		case 'A': return 1; // fly
		case 'F': return 3; // float/hover
		default:  return 0; // land
	}
}

static uint8_t parse_size(char c)
{
	switch (c)
	{
		case 'S': return 0;
		case 'M': return 1;
		case 'L': return 2;
		case 'H': return 3;
		default:  return 1;
	}
}

static uint8_t parse_alignment(char c)
{
	switch (c)
	{
		case 'G': return 0; // good
		case 'E': return 2; // evil
		default:  return 1; // neutral
	}
}

static uint8_t parse_race(char c)
{
	// Map race character to index (arbitrary but consistent)
	static const char race_chars[] = "HDUBMOEWKVAFNXCLR";
	for (int i = 0; race_chars[i]; i++)
		if (race_chars[i] == c) return (uint8_t)i;
	return 0xFF;
}

// ---------------------------------------------------------------------------
// Parse [ARMIES] + [HEROES] sections
// ---------------------------------------------------------------------------

static uint32_t parse_army_section(const char* text, const char* section,
                                   baked_unit_def* defs, uint32_t offset, uint32_t max)
{
	const char* cursor = find_section(text, section);
	if (!cursor) return 0;

	char line[512];
	uint32_t count = 0;

	while (get_next_line(&cursor, line, sizeof(line)) > 0)
	{
		if (offset + count >= max) break;
		int len = (int)strlen(line);
		if (len < ARMYNAME_INDEX) continue;

		baked_unit_def* d = &defs[offset + count];
		memset(d, 0, sizeof(*d));

		// Code (4 chars at column 4)
		read_str_at(line, CODE_INDEX, 4, len, d->code, sizeof(d->code));
		if (d->code[0] == '\0') continue;

		// Status flags
		if (len > STATUSFLAG_INDEX + 7)
		{
			d->size_class = parse_size(line[STATUSFLAG_INDEX]);
			d->alignment = parse_alignment(line[STATUSFLAG_INDEX + 1]);
			d->race = parse_race(line[STATUSFLAG_INDEX + 2]);
			d->movement_type = parse_movement(line[STATUSFLAG_INDEX + 4]);
			d->is_missile = (len > STATUSFLAG_INDEX + 7 && line[STATUSFLAG_INDEX + 7] == 'M') ? 1 : 0;
		}

		// Missile code
		if (len > MISSILE_INDEX + 5 && line[MISSILE_INDEX] != '-')
		{
			read_str_at(line, MISSILE_INDEX + 2, 4, len, d->missile_code, sizeof(d->missile_code));
		}

		// Mana
		if (len > MANA_INDEX + 6 && line[MANA_INDEX] != '-')
		{
			d->max_mana = (uint16_t)read_int_at(line, MANA_INDEX, 3, len);
			d->start_mana = (uint16_t)read_int_at(line, MANA_INDEX + 4, 3, len);
		}

		// General flag
		if (len > GENERAL_INDEX && line[GENERAL_INDEX] != '-')
			d->is_general = 1;

		// Name (rest of line from column 62)
		read_rest(line, ARMYNAME_INDEX, len, d->name, sizeof(d->name));

		count++;
	}
	return count;
}

// ---------------------------------------------------------------------------
// Parse [RULES] section and merge stats into existing defs
// ---------------------------------------------------------------------------

static void parse_rules_section(const char* text, baked_unit_def* defs, uint32_t count)
{
	const char* cursor = find_section(text, "[RULES]");
	if (!cursor) return;

	char line[512];
	while (get_next_line(&cursor, line, sizeof(line)) > 0)
	{
		int len = (int)strlen(line);
		if (len < 10) continue;

		char code[8] = {0};
		read_str_at(line, RULES_CODE_INDEX, 4, len, code, sizeof(code));
		if (code[0] == '\0') continue;

		// Find matching unit def (use first match — first race's rules as base stats)
		baked_unit_def* d = NULL;
		for (uint32_t i = 0; i < count; i++)
		{
			if (strcmp(defs[i].code, code) == 0)
			{
				// Only apply if stats haven't been set yet (first rules entry wins)
				if (defs[i].hits == 0)
				{
					d = &defs[i];
					break;
				}
			}
		}
		if (!d) continue;

		d->cost_gold    = (uint16_t)read_int_at(line, RULES_GOLD_INDEX, 5, len);
		d->cost_metal   = (uint16_t)read_int_at(line, RULES_METAL_INDEX, 5, len);
		d->cost_stone   = (uint16_t)read_int_at(line, RULES_STONE_INDEX, 5, len);
		d->cost_crystal = (uint16_t)read_int_at(line, RULES_CRYST_INDEX, 5, len);
		d->produce_time = (uint16_t)read_int_at(line, RULES_TIME_INDEX, 4, len);
		d->combat       = (uint16_t)read_int_at(line, RULES_COMBAT_INDEX, 4, len);
		d->hits         = (uint16_t)read_int_at(line, RULES_HITS_INDEX, 4, len);
		d->speed        = (uint16_t)read_int_at(line, RULES_SPEED_INDEX, 4, len);
		d->damage       = (uint16_t)read_int_at(line, RULES_DMG_INDEX, 4, len);
		d->damage_range = (uint16_t)read_int_at(line, RULES_RANGE_INDEX, 4, len);
	}
}

// ---------------------------------------------------------------------------
// Public: merge [RULES] from Building.cfg
// ---------------------------------------------------------------------------

void cfg_merge_rules(const char* bldg_cfg_text, baked_unit_def* defs, uint32_t count)
{
	parse_rules_section(bldg_cfg_text, defs, count);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

uint32_t cfg_parse_armies(const char* cfg_text, uint32_t cfg_size,
                          baked_unit_def** out)
{
	(void)cfg_size;

	// Count entries
	int army_count = count_section_lines(cfg_text, "[ARMIES]");
	int hero_count = count_section_lines(cfg_text, "[HEROES]");
	int total = army_count + hero_count;
	if (total == 0) return 0;

	baked_unit_def* defs = calloc(total, sizeof(baked_unit_def));
	if (!defs) return 0;

	// Parse armies and heroes
	uint32_t armies = parse_army_section(cfg_text, "[ARMIES]", defs, 0, total);
	uint32_t heroes = parse_army_section(cfg_text, "[HEROES]", defs, armies, total);
	uint32_t count = armies + heroes;

	// Parse rules to fill in stats
	parse_rules_section(cfg_text, defs, count);

	fprintf(stderr, "[cfg] Parsed %u armies + %u heroes = %u units\n", armies, heroes, count);

	// Apply default stats for units without [RULES] data
	for (uint32_t i = 0; i < count; i++)
	{
		baked_unit_def* d = &defs[i];
		if (d->hits == 0)
		{
			// Reasonable defaults scaled by size class
			uint16_t size_mult = (uint16_t)(1 + d->size_class); // 1-4
			d->hits = (uint16_t)(30 * size_mult);
			d->combat = (uint16_t)(30 + 10 * size_mult);
			d->speed = 60;
			d->damage = (uint16_t)(8 * size_mult);
			d->armor = (uint16_t)(2 * size_mult);
			d->damage_range = 50;
			d->view = 100;
			d->cost_gold = (uint16_t)(30 * size_mult);
			d->cost_metal = (uint16_t)(10 * size_mult);
			d->produce_time = (uint16_t)(8 + 4 * size_mult);
		}
	}

	*out = defs;
	return count;
}

bool cfg_write_units_bin(const char* path, const baked_unit_def* defs, uint32_t count)
{
	FILE* f = fopen(path, "wb");
	if (!f) return false;

	baked_header hdr = {
		.magic = BAKED_UNITS_MAGIC,
		.version = BAKED_VERSION,
		.record_count = count,
		.record_size = sizeof(baked_unit_def),
	};

	fwrite(&hdr, sizeof(hdr), 1, f);
	fwrite(defs, sizeof(baked_unit_def), count, f);
	fclose(f);

	fprintf(stderr, "[cfg] Wrote %s: %u records (%zu bytes each)\n",
	        path, count, sizeof(baked_unit_def));
	return true;
}
