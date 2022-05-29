#pragma once

#include "../engine/types.h"

enum command_type
{
    COMMAND_MOVE,
};

typedef struct command_move_t
{
    uint32_t entity_count;
    int16_t x, y;
} command_move_t;

typedef struct command_t
{
    uint8_t type;
    union
    {
        command_move_t move;
    };
} command_t;