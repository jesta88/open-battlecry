#include <engine/config.h>
#include <engine/log.h>
#include "core/hash.h"
#include "core/string.inl"
#include <stdio.h>
#include <assert.h>

enum
{
    MAX_CLIENT_CONFIGS = 128,
    MAX_SERVER_CONFIGS = 64,
    MAX_GAME_CONFIGS = 128,

    // Private flags
    CONFIG_INT = 1 << 0,
    CONFIG_FLOAT = 1 << 1,
    CONFIG_BOOL = 1 << 2,
    CONFIG_MODIFIED = 1 << 3,
};

static uint8_t client_config_count;
static uint8_t server_config_count;
static uint8_t game_config_count;
static struct config client_configs[MAX_CLIENT_CONFIGS];
static struct config server_configs[MAX_SERVER_CONFIGS];
static struct config game_configs[MAX_GAME_CONFIGS];

static struct config* find(uint32_t hash, struct config* configs, uint8_t config_count)
{
    for (int i = 0; i < config_count; i++)
    {
        if (configs[i].name_hash == hash)
            return &configs[i];
    }
    return NULL;
}

static bool find_or_add(const char* name, struct config** config, uint8_t flags)
{
    char type = name[0];
    uint32_t hash = hash_string(name);
    bool is_new;

    switch (type)
    {
        case 'c':
            *config = find(hash, client_configs, client_config_count);
            is_new = *config == NULL;
            *config = &client_configs[client_config_count++];
            break;
        case 's':
            *config = find(hash, server_configs, server_config_count);
            is_new = *config == NULL;
            *config = &server_configs[server_config_count++];
            break;
        case 'g':
            *config = find(hash, game_configs, game_config_count);
            is_new = *config == NULL;
            *config = &game_configs[game_config_count++];
            break;
        default:
            log_error("Invalid config name: %s\n", name);
            return true;
    }

    if (is_new)
    {
        string_copy((*config)->name, name, MAX_CONFIG_NAME_LENGTH);
        (*config)->name_hash = hash;
        (*config)->flags = flags;
    }

    return is_new;
}

struct config* config_get_int(const char* name, int32_t value, uint8_t flags)
{
    struct config* config = NULL;
    bool isNew = find_or_add(name, &config, flags);
    assert(config);

    if (isNew)
    {
        config->int_value = value;
        config->flags |= CONFIG_INT;
    }
    return config;
}

struct config* config_get_float(const char* name, float value, uint8_t flags)
{
    struct config* config = NULL;
    bool isNew = find_or_add(name, &config, flags);
    assert(config != NULL);

    if (isNew)
    {
        config->float_value = value;
        config->flags |= CONFIG_FLOAT;
    }
    return config;
}

struct config* config_get_bool(const char* name, bool value, uint8_t flags)
{
    struct config* config = NULL;
    bool isNew = find_or_add(name, &config, flags);
    assert(config != NULL);

    if (isNew)
    {
        config->bool_value = value;
        config->flags |= CONFIG_BOOL;
    }
    return config;
}

void config_load(const char* file_name)
{
    FILE* file = fopen(file_name, "r");
    if (file == NULL)
    {
        log_info("Could not find config file: %s", file_name);
        return;
    }

    fclose(file);
}

void config_save(const char* file_name)
{
    FILE* file = fopen(file_name, "w");
    if (file == NULL)
    {
        log_error("Failed to open file: %s", file_name);
        return;
    }

//    char buffer[WB_MAX_CONFIGS * 60];
//    int buffer_length = 0;
//    for (int i = 0; i < config_count; i++)
//    {
//        if (!configs[i]->save)
//            continue;
//
//        char type;
//        char value[28];
//        switch (configs[i]->type)
//        {
//            case WB_CVAR_INT:
//                type = 'i';
//                sprintf(value, "%d", configs[i]->int_value);
//                break;
//            case WB_CVAR_FLOAT:
//                type = 'f';
//                sprintf(value, "%.3f", configs[i]->float_value);
//                break;
//            case WB_CVAR_BOOL:
//                type = 'b';
//                sprintf(value, "%s", configs[i]->bool_value ? "true" : "false");
//                break;;
//        }
//        buffer_length += sprintf(buffer, "%s:%c=%s\n", configs[i]->name, type, value);
//    }
//
//    SDL_RWwrite(file, buffer, 1, buffer_length);
    fclose(file);
}
