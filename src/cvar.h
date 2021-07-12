#pragma once

#include "types.h"

typedef enum WbCvarType
{
    WB_CVAR_INT,
    WB_CVAR_FLOAT,
    WB_CVAR_BOOL,
    WB_CVAR_STRING
} WbCvarType;

typedef struct WbCvar
{
    char name[32];
    bool save;
    uint8_t index;
    uint8_t type;
    union
    {
        int32_t int_value;
        float float_value;
        bool bool_value;
        char string_value[28];
    };
} WbCvar;

_Static_assert(sizeof(WbCvar) == 64, "WbCvar must be 64 bytes.");

void wbCvarLoadFromFile(const char* file_name);
void wbCvarSaveToFile(const char* file_name);

void wbCvarRegisterInt(WbCvar* cvar, const char* name, int32_t int_value, bool save);
void wbCvarRegisterFloat(WbCvar* cvar, const char* name, float float_value, bool save);
void wbCvarRegisterBool(WbCvar* cvar, const char* name, bool bool_value, bool save);
void wbCvarRegisterString(WbCvar* cvar, const char* name, const char* string_value, bool save);
