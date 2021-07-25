#pragma once

#include <stdint.h>
#include <stdbool.h>

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

#ifndef min
    #define min(a, b) \
        ({  typeof (a) _a = (a); \
            typeof (b) _b = (b); \
            _a < _b ? _a : _b; })
#endif

#ifndef max
    #define max(a, b) \
    ({  typeof (a) _a = (a); \
        typeof (b) _b = (b); \
        _a > _b ? _a : _b; })
#endif

bool rect_contains_point(rect_t rect, point_t point);
void rects_contain_points(const rect_t rects[4], const point_t points[4], bool results[4]);