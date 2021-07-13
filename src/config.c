#define _CRT_SECURE_NO_WARNINGS

#include "config.h"
#include "hash_map.h"
#include "log.h"
#include <SDL2/SDL_rwops.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

enum
{
    WB_MAX_CONFIGS = 256
};

static uint8_t config_count;
static WbConfig* configs[WB_MAX_CONFIGS];

static void saveToFile(const char* file_name);

void wbConfigLoad(const char* file_name)
{
    SDL_RWops* file = SDL_RWFromFile(file_name, "r");
    if (file == NULL)
    {

        saveToFile(file_name);
    }

    SDL_RWclose(file);
}

static WbConfig* findCvar(const char* name)
{
    // TODO: Profile and replace with hash map?
    for (int i = 0; i < config_count; i++)
    {
        if (strcmp(name, configs[i]->name) == 0)
        {
            return configs[i];
        }
    }

    return NULL;
}

WbConfig* wbCvarGetInt(const char* name, int32_t value, bool save)
{
    WbConfig* cvar = findCvar(name);

    if (cvar != NULL)
    {
        cvar->save = save;
        cvar->type = WB_CVAR_INT;
        cvar->int_value = value;
    }

    return NULL;
}

void wbCvarRegisterInt(WbConfig* cvar, const char* name, int32_t int_value, bool save)
{
    strcpy(cvar->name, name);
    cvar->save = save;
    cvar->index = config_count++;
    cvar->type = WB_CVAR_INT;
    cvar->int_value = int_value;

    configs[cvar->index] = cvar;
}

void wbCvarRegisterFloat(WbConfig* cvar, const char* name, float float_value, bool save)
{
    strcpy(cvar->name, name);
    cvar->save = save;
    cvar->index = config_count++;
    cvar->type = WB_CVAR_FLOAT;
    cvar->float_value = float_value;

    configs[cvar->index] = cvar;
}

void wbCvarRegisterBool(WbConfig* cvar, const char* name, bool bool_value, bool save)
{
    strcpy(cvar->name, name);
    cvar->save = save;
    cvar->index = config_count++;
    cvar->type = WB_CVAR_BOOL;
    cvar->bool_value = bool_value;

    configs[cvar->index] = cvar;
}

static void saveToFile(const char* file_name)
{
    SDL_RWops* file = SDL_RWFromFile(file_name, "w");
    if (file == NULL)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }

    char buffer[WB_MAX_CONFIGS * 64];
    int buffer_length = 0;
    for (int i = 0; i < config_count; i++)
    {
        if (!configs[i]->save)
            continue;

        char type;
        char value[28];
        switch (configs[i]->type)
        {
            case WB_CVAR_INT:
                type = 'i';
                sprintf(value, "%d", configs[i]->int_value);
                break;
            case WB_CVAR_FLOAT:
                type = 'f';
                sprintf(value, "%.3f", configs[i]->float_value);
                break;
            case WB_CVAR_BOOL:
                type = 'b';
                sprintf(value, "%s", configs[i]->bool_value ? "true" : "false");
                break;;
        }
        buffer_length += sprintf(buffer, "%s:%c=%s\n", configs[i]->name, type, value);
    }

    SDL_RWwrite(file, buffer, 1, buffer_length);
    SDL_RWclose(file);
}
