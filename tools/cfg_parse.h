#pragma once

#include "baked_format.h"
#include <stdbool.h>
#include <stdint.h>

// Parse Army.cfg from raw text data and produce baked unit definitions.
// Returns number of units parsed, or 0 on failure.
// Caller must free(*out) when done.
uint32_t cfg_parse_armies(const char* cfg_text, uint32_t cfg_size,
                          baked_unit_def** out);

// Merge [RULES] stats from Building.cfg into existing unit defs (matched by code).
void cfg_merge_rules(const char* bldg_cfg_text, baked_unit_def* defs, uint32_t count);

// Write baked unit definitions to a binary file.
bool cfg_write_units_bin(const char* path, const baked_unit_def* defs, uint32_t count);
