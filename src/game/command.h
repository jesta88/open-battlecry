#pragma once

#include "../engine/std.h"

enum command_type
{
    COMMAND_MOVE,
};

typedef struct command_move_t
{
    u32 entity_count;
    s16 x, y;
} command_move_t;

typedef struct command_t
{
    u8 type;
    union
    {
        command_move_t move;
    };
} command_t;