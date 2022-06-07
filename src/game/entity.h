#pragma once

#include "../engine/types.h"

enum
{
    MAX_UNITS = 4096,
};

typedef struct position_t
{
    s16 x;
    s16 y;
    u32 version;
} position_t;

typedef struct destination_t
{
    s16 x;
    s16 y;
} destination_t;

typedef struct sprite_t
{
    u16 texture_id;
    u8 direction;
    u8 frame;
} sprite_t;

typedef struct health_t
{
    u16 amount;
} health_t;

typedef struct game_state_t
{
    position_t positions[MAX_UNITS];
    destination_t destinations[MAX_UNITS];
    sprite_t sprites[MAX_UNITS];
    health_t healths[MAX_UNITS];
} game_state_t;