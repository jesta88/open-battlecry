#pragma once

#include <stdint.h>
#include <stdbool.h>

enum
{
    WB_MAX_CONFIG_NAME_LENGTH = 23  // Ensures that WbConfig is 32 bytes.
};

typedef enum WbConfigFlags
{
    WB_CONFIG_SAVE = 1 << 4,
    WB_CONFIG_DEPRECATED = 1 << 5,
} WbConfigFlags;

typedef struct WbConfig
{
    uint32_t name_hash;
    char name[WB_MAX_CONFIG_NAME_LENGTH];
    uint8_t flags;
    union
    {
        int32_t int_value;
        float float_value;
        bool bool_value;
    };
} WbConfig;

_Static_assert(sizeof(WbConfig) == 32, "WbConfig must be 32 bytes.");

WbConfig* wbConfigInt(const char* name, int32_t value, uint8_t flags);
WbConfig* wbConfigFloat(const char* name, float value, uint8_t flags);
WbConfig* wbConfigBool(const char* name, bool value, uint8_t flags);

void wbConfigLoad(const char* file_name);
void wbConfigSave(const char* file_name);
