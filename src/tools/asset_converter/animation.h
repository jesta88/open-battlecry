#pragma once

#include "std.h"

typedef enum
{
    WB_ANIMATION_TYPE_STAND,
    WB_ANIMATION_TYPE_WALK,
    WB_ANIMATION_TYPE_FIGHT,
    WB_ANIMATION_TYPE_DIE,
    WB_ANIMATION_TYPE_AMBIENT,
    WB_ANIMATION_TYPE_SPECIAL,
    WB_ANIMATION_TYPE_CONVERT,
    WB_ANIMATION_TYPE_SPELL,
    WB_ANIMATION_TYPE_INTERFACE,
    WB_ANIMATION_TYPE_COUNT
} wb_animation_type;

typedef struct
{
    bool used;
    u8 frame_count;
    u8 effects[2];
    s16 origin_x;
    s16 origin_y;
    s16 width;
    s16 height;
    s16 effect_origin_x;
    s16 effect_origin_y;
    s16 selection_origin_x;
    s16 selection_origin_y;
    s16 selection_sie;
    s16 effect_type;
    s16 effect_activation;
} wb_animation;
