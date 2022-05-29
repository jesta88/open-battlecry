#pragma once

#include "../engine/types.h"

enum
{
    MAX_UNITS = 4096,
};

typedef struct position_t
{
    int16_t x;
    int16_t y;
    uint32_t version;
} position_t;

typedef struct destination_t
{
    int16_t x;
    int16_t y;
} destination_t;

typedef struct sprite_t
{
    uint16_t texture_id;
    uint8_t direction;
    uint8_t frame;
} sprite_t;

typedef struct health_t
{
    uint16_t amount;
} health_t;

typedef struct game_state_t
{
    position_t positions[MAX_UNITS];
    destination_t destinations[MAX_UNITS];
    sprite_t sprites[MAX_UNITS];
    health_t healths[MAX_UNITS];
} game_state_t;