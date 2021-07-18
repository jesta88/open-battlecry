#pragma once

#include <stdint.h>

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
    int32_t x, y;
} point_t;

typedef struct rect_t
{
    int32_t x, y;
    int32_t w, h;
} rect_t;
