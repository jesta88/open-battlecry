#pragma once

#include "types.h"

typedef enum WbConfigFlags
{
    // Flags 1, 2 and 4 are reserved for config types
    WB_CONFIG_SAVE = 1 << 3,
    WB_CONFIG_DEPRECATED = 1 << 4,
} WbConfigFlags;

WbConfig* wbConfigInt(const char* name, int32_t value, uint8_t flags);
WbConfig* wbConfigFloat(const char* name, float value, uint8_t flags);
WbConfig* wbConfigBool(const char* name, bool value, uint8_t flags);

void wbConfigLoad(const char* file_name);
void wbConfigSave(const char* file_name);
