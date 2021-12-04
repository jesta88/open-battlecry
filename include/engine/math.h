#pragma once

#include "types.h"

#define WAR_PI32 3.14159265359f
#define WAR_PI 3.14159265358979323846

#define WAR_MIN(a, b) ((a) > (b) ? (b) : (a))
#define WAR_MAX(a, b) ((a) < (b) ? (b) : (a))
#define WAR_ABS(a) ((a) > 0 ? (a) : -(a))
#define WAR_MOD(a, m) (((a) % (m)) >= 0 ? ((a) % (m)) : (((a) % (m)) + (m)))
#define WAR_SQUARE(x) ((x) * (x)

typedef struct WarVec2
{
    float x, y;
} WARVec2;

typedef struct WARVec3
{
    float x, y, z;
} WARVec3;

typedef struct WARVec4
{
    float x, y, z, w;
} WARVec4;

typedef struct WARPoint
{
    int16_t x, y;
} WARPoint;

bool rect_contains_point(rect_t rect, point_t point);