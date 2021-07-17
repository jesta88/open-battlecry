#pragma once

#include <stdint.h>
#include <stdbool.h>

enum
{
    MAX_CONFIG_NAME_LENGTH = 23  // Ensures that config_t is 32 bytes.
};

enum config_flags
{
    CONFIG_SAVE = 1 << 4,
    CONFIG_DEPRECATED = 1 << 5,
};

struct config
{
    uint32_t name_hash;
    char name[MAX_CONFIG_NAME_LENGTH];
    uint8_t flags;
    union
    {
        int32_t int_value;
        float float_value;
        bool bool_value;
    };
};

_Static_assert(sizeof(struct config) == 32, "config must be 32 bytes.");

struct config* config_get_int(const char* name, int32_t value, uint8_t flags);
struct config* config_get_float(const char* name, float value, uint8_t flags);
struct config* config_get_bool(const char* name, bool value, uint8_t flags);

void config_load(const char* file_name);
void config_save(const char* file_name);
