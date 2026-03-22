#pragma once

#include <stdbool.h>

#define RG_MAX_CONFIG_ENTRIES 64
#define RG_MAX_KEY_LENGTH 32
#define RG_MAX_VALUE_LENGTH 64

typedef struct ConfigEntry
{
	char key[RG_MAX_KEY_LENGTH];
	char value[RG_MAX_VALUE_LENGTH];
} ConfigEntry;

typedef struct Config
{
	ConfigEntry entries[RG_MAX_CONFIG_ENTRIES];
	int count;
} Config;

// Load config from a file (returns non-zero on failure)
int wc_config_load(Config* config, const char* filename);

// Save config to a file (returns non-zero on failure)
int wc_config_save(const Config* config, const char* filename);

// Get a string value (returns NULL if not found)
const char* wc_config_get_str(const Config* config, const char* key, const char* default_value);

// Get an integer value (returns default if not found or invalid)
int wc_config_get_int(const Config* config, const char* key, int default_value);

// Get a boolean value (returns default if not found or invalid)
bool wc_config_get_bool(const Config* config, const char* key, bool default_value);

// Set a string value (returns non-zero if key is too long or table is full)
int wc_config_set_str(Config* config, const char* key, const char* value);

// Set an integer value (returns non-zero if key is too long or table is full)
int wc_config_set_int(Config* config, const char* key, int value);

// Set a boolean value (returns non-zero if key is too long or table is full)
int wc_config_set_bool(Config* config, const char* key, bool value);
