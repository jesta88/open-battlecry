#pragma once

#include "../../../include/engine/types.h"

typedef struct vec2_t
{
    float x, y;
} vec2_t;

typedef struct vec3_t
{
    float x, y, z;
} vec3_t;

typedef struct vec4_t
{
    float x, y, z, w;
} vec4_t;

typedef struct point_t
{
    int16_t x, y;
} point_t;

typedef struct rect_t
{
    int16_t x, y;
    int16_t w, h;
} rect_t;

bool rect_contains_point(rect_t rect, point_t point);