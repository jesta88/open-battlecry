#pragma once

#include <stdint.h>

enum
{
    MAX_SELECTION = 512,
};

typedef struct entity_t
{
    uint32_t id;
    uint32_t version;
} entity_t;