#define _CRT_SECURE_NO_WARNINGS

#include "cvar.h"
#include "hash_map.h"
#include "log.h"
#include <SDL2/SDL_rwops.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

enum
{
    WB_MAX_CVARS = 256
};

static uint8_t cvar_count;
static const WbCvar* cvars[WB_MAX_CVARS];

static void saveToFile(const char* file_name);
static void initCvar(WbCvar* cvar, const char* name, bool save);

void wbCvarRegisterInt(WbCvar* cvar, const char* name, int32_t int_value, bool save)
{
    strcpy(cvar->name, name);
    cvar->save = save;
    cvar->index = cvar_count++;
    cvar->type = WB_CVAR_INT;
    cvar->int_value = int_value;

    cvars[cvar->index] = cvar;
}

void wbCvarRegisterFloat(WbCvar* cvar, const char* name, float float_value, bool save)
{
    strcpy(cvar->name, name);
    cvar->save = save;
    cvar->index = cvar_count++;
    cvar->type = WB_CVAR_FLOAT;
    cvar->float_value = float_value;

    cvars[cvar->index] = cvar;
}

void wbCvarRegisterBool(WbCvar* cvar, const char* name, bool bool_value, bool save)
{
    strcpy(cvar->name, name);
    cvar->save = save;
    cvar->index = cvar_count++;
    cvar->type = WB_CVAR_BOOL;
    cvar->bool_value = bool_value;

    cvars[cvar->index] = cvar;
}

void wbCvarRegisterString(WbCvar* cvar, const char* name, const char* string_value, bool save)
{
    strcpy(cvar->name, name);
    cvar->save = save;
    cvar->index = cvar_count++;
    cvar->type = WB_CVAR_STRING;
    strcpy(cvar->string_value, string_value);

    cvars[cvar->index] = cvar;
}

void wbCvarLoadFromFile(const char* file_name)
{
    SDL_RWops* file = SDL_RWFromFile(file_name, "r");
    if (file == NULL)
    {
        saveToFile(file_name);
    }

    SDL_RWclose(file);
}

void wbCvarSaveToFile(const char* file_name)
{
    saveToFile(file_name);
}

static void saveToFile(const char* file_name)
{
    SDL_RWops* file = SDL_RWFromFile(file_name, "w");
    if (file == NULL)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }

    char buffer[WB_MAX_CVARS * 64];
    int buffer_length = 0;
    for (int i = 0; i < cvar_count; i++)
    {
        if (!cvars[i]->save)
            continue;

        char type;
        char value[28];
        switch (cvars[i]->type)
        {
            case WB_CVAR_INT:
                type = 'i';
                sprintf(value, "%d", cvars[i]->int_value);
                break;
            case WB_CVAR_FLOAT:
                type = 'f';
                sprintf(value, "%.3f", cvars[i]->float_value);
                break;
            case WB_CVAR_BOOL:
                type = 'b';
                sprintf(value, "%s", cvars[i]->bool_value ? "true" : "false");
                break;;
            case WB_CVAR_STRING:
                type = 's';
                sprintf(value, "%s", cvars[i]->string_value);
                break;
        }
        buffer_length += sprintf(buffer, "%s:%c=%s\n", cvars[i]->name, type, value);
    }

    SDL_RWwrite(file, buffer, 1, buffer_length);
    SDL_RWclose(file);
}
