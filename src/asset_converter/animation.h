#pragma once

#include <core/types.h>

typedef enum
{
    ANIMATION_TYPE_STAND,
    ANIMATION_TYPE_WALK,
    ANIMATION_TYPE_FIGHT,
    ANIMATION_TYPE_DIE,
    ANIMATION_TYPE_AMBIENT,
    ANIMATION_TYPE_SPECIAL,
    ANIMATION_TYPE_CONVERT,
    ANIMATION_TYPE_SPELL,
    ANIMATION_TYPE_INTERFACE,
    ANIMATION_TYPE_COUNT
} animation_type_t;

typedef struct
{
    bool used;
    uint8_t frame_count;
    uint8_t effects[2];
    int16_t origin_x;
    int16_t origin_y;
    int16_t width;
    int16_t height;
    int16_t effect_origin_x;
    int16_t effect_origin_y;
    int16_t selection_origin_x;
    int16_t selection_origin_y;
    int16_t selection_sie;
    int16_t effect_type;
    int16_t effect_activation;
} animation_t;

typedef struct
{
    animation_t animations[ANIMATION_TYPE_COUNT];
} animation_data_t;

