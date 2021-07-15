#include "config.h"
#include "hash.h"
#include "string.inl"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

enum
{
    WB_MAX_CLIENT_CONFIGS = 128,
    WB_MAX_SERVER_CONFIGS = 64,
    WB_MAX_GAME_CONFIGS = 128,

    // Private flags
    WB_CONFIG_INT = 1 << 0,
    WB_CONFIG_FLOAT = 1 << 1,
    WB_CONFIG_BOOL = 1 << 2,
    WB_CONFIG_MODIFIED = 1 << 3,
};

static uint8_t client_config_count;
static uint8_t server_config_count;
static uint8_t game_config_count;
static WbConfig client_configs[WB_MAX_CLIENT_CONFIGS];
static WbConfig server_configs[WB_MAX_SERVER_CONFIGS];
static WbConfig game_configs[WB_MAX_GAME_CONFIGS];

static WbConfig* find(uint32_t hash, WbConfig* configs, uint8_t config_count)
{
    for (int i = 0; i < config_count; i++)
    {
        if (configs[i].name_hash == hash)
            return &configs[i];
    }
    return NULL;
}

static bool findOrAdd(const char* name, WbConfig** config, uint8_t flags)
{
    char type = name[0];
    uint32_t hash = wbHashString32(name);
    bool isNew;

    switch (type)
    {
        case 'c':
            *config = find(hash, client_configs, client_config_count);
            isNew = *config == NULL;
            *config = &client_configs[client_config_count++];
            break;
        case 's':
            *config = find(hash, server_configs, server_config_count);
            isNew = *config == NULL;
            *config = &server_configs[server_config_count++];
            break;
        case 'g':
            *config = find(hash, game_configs, game_config_count);
            isNew = *config == NULL;
            *config = &game_configs[game_config_count++];
            break;
        default:
            s_log_error("Invalid config name: %s\n", name);
            return true;
    }

    if (isNew)
    {
        wbStrncpy((*config)->name, name, WB_MAX_CONFIG_NAME_LENGTH);
        (*config)->name_hash = hash;
        (*config)->flags = flags;
    }

    return isNew;
}

WbConfig* wbConfigInt(const char* name, int32_t value, uint8_t flags)
{
    WbConfig* config = NULL;
    bool isNew = findOrAdd(name, &config, flags);
    assert(config);

    if (isNew)
    {
        config->int_value = value;
        config->flags |= WB_CONFIG_INT;
    }
    return config;
}

WbConfig* wbConfigFloat(const char* name, float value, uint8_t flags)
{
    WbConfig* config = NULL;
    bool isNew = findOrAdd(name, &config, flags);
    assert(config != NULL);

    if (isNew)
    {
        config->float_value = value;
        config->flags |= WB_CONFIG_FLOAT;
    }
    return config;
}

WbConfig* wbConfigBool(const char* name, bool value, uint8_t flags)
{
    WbConfig* config = NULL;
    bool isNew = findOrAdd(name, &config, flags);
    assert(config != NULL);

    if (isNew)
    {
        config->bool_value = value;
        config->flags |= WB_CONFIG_BOOL;
    }
    return config;
}

void wbConfigLoad(const char* file_name)
{
    FILE* file = fopen(file_name, "r");
    if (file == NULL)
    {
        s_log_info("Could not find config file: %s", file_name);
        return;
    }

    fclose(file);
}

void wbConfigSave(const char* file_name)
{
    FILE* file = fopen(file_name, "w");
    if (file == NULL)
    {
        s_log_error("Failed to open file: %s", file_name);
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
