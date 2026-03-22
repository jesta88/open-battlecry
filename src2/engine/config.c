#include <engine/config.h>

#include <SDL3/SDL_iostream.h>
#include <stdlib.h>

int wc_config_load(Config* config, const char* filename)
{
	SDL_IOStream* file = SDL_IOFromFile(filename, "r");
	if (!file)
		return -1;

	char line[RG_MAX_KEY_LENGTH + RG_MAX_VALUE_LENGTH + 2]; // Key + '=' + Value + '\0'
	config->count = 0;

	while (SDL_ReadIO(file, line, sizeof(line)))
	{
		char* eq = SDL_strchr(line, '=');
		if (!eq)
			continue; // Skip malformed lines

		*eq = '\0'; // Split key and value

		// Trim whitespace from key and value
		char* key_trim = line;
		while (*key_trim == ' ' || *key_trim == '\t')
			key_trim++;
		char* key_end = key_trim + SDL_strlen(key_trim) - 1;
		while (key_end > key_trim && (*key_end == ' ' || *key_end == '\t' || *key_end == '\n'))
			key_end--;
		*(key_end + 1) = '\0';

		char* val_trim = eq + 1;
		while (*val_trim == ' ' || *val_trim == '\t')
			val_trim++;
		char* val_end = val_trim + SDL_strlen(val_trim) - 1;
		while (val_end > val_trim && (*val_end == ' ' || *val_end == '\t' || *val_end == '\n'))
			val_end--;
		*(val_end + 1) = '\0';

		if (SDL_strlen(key_trim) >= RG_MAX_KEY_LENGTH || SDL_strlen(val_trim) >= RG_MAX_VALUE_LENGTH)
		{
			continue; // Skip if too long
		}

		SDL_strlcpy(config->entries[config->count].key, key_trim, RG_MAX_KEY_LENGTH);
		SDL_strlcpy(config->entries[config->count].value, val_trim, RG_MAX_VALUE_LENGTH);
		config->count++;
	}

	SDL_CloseIO(file);
	return 0;
}

int wc_config_save(const Config* config, const char* filename)
{
	SDL_IOStream* file = SDL_IOFromFile(filename, "w");
	if (!file)
		return -1;

	char line[RG_MAX_KEY_LENGTH + RG_MAX_VALUE_LENGTH + 2]; // Key + '=' + Value + '\n' + '\0'

	for (int i = 0; i < config->count; i++)
	{
		const int length = SDL_snprintf(line, sizeof(line), "%s=%s\n", config->entries[i].key, config->entries[i].value);
		if (length <= 0 || SDL_WriteIO(file, line, length) != length)
		{
			SDL_CloseIO(file);
			return -1;
		}
	}

	SDL_CloseIO(file);
	return 0;
}

const char* wc_config_get_str(const Config* config, const char* key, const char* default_value)
{
	for (int i = 0; i < config->count; i++)
	{
		if (SDL_strcmp(config->entries[i].key, key) == 0)
		{
			return config->entries[i].value;
		}
	}
	return default_value;
}

int wc_config_get_int(const Config* config, const char* key, int default_value)
{
	const char* val = wc_config_get_str(config, key, NULL);
	if (!val)
		return default_value;
	return SDL_atoi(val);
}

bool wc_config_get_bool(const Config* config, const char* key, bool default_value)
{
	const char* val = wc_config_get_str(config, key, NULL);
	if (!val)
		return default_value;
	return SDL_strcmp(val, "true") == 0 || SDL_strcmp(val, "1") == 0;
}

int wc_config_set_str(Config* config, const char* key, const char* value)
{
	if (SDL_strlen(key) >= RG_MAX_KEY_LENGTH || SDL_strlen(value) >= RG_MAX_VALUE_LENGTH) {
		return -1;
	}

	// Update existing key if found
	for (int i = 0; i < config->count; i++) {
		if (SDL_strcmp(config->entries[i].key, key) == 0) {
			SDL_strlcpy(config->entries[i].value, value, RG_MAX_VALUE_LENGTH);
			return 0;
		}
	}

	// Add new entry if space available
	if (config->count >= RG_MAX_CONFIG_ENTRIES) {
		return -1;
	}

	SDL_strlcpy(config->entries[config->count].key, key, RG_MAX_KEY_LENGTH);
	SDL_strlcpy(config->entries[config->count].value, value, RG_MAX_VALUE_LENGTH);
	config->count++;
	return 0;
}

int wc_config_set_int(Config* config, const char* key, const int value)
{
	char str_val[16];
	SDL_snprintf(str_val, sizeof(str_val), "%d", value);
	return wc_config_set_str(config, key, str_val);
}

int wc_config_set_bool(Config* config, const char* key, const bool value)
{
	return wc_config_set_str(config, key, value ? "true" : "false");
}
