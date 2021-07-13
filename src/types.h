#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct WbVec2
{
    float x, y;
} WbVec2;

typedef struct WbVec3
{
    float x, y, z;
} WbVec3;

typedef struct WbVec4
{
    float x, y, z, w;
} WbVec4;

typedef struct WbPoint
{
    int32_t x, y;
} WbPoint;

typedef struct WbRect
{
    int32_t x, y;
    int32_t w, h;
} WbRect;

typedef struct WbTempAllocator
{
    uint8_t* buffer;
    uint32_t size;
    uint32_t offset;
} WbTempAllocator;

typedef enum WbCvarType
{
    WB_CVAR_INT,
    WB_CVAR_FLOAT,
    WB_CVAR_BOOL,
    WB_CVAR_STRING
} WbCvarType;

typedef struct WbConfig
{
    char name[25];
    bool save;
    uint8_t type;
    uint8_t index;
    union
    {
        int32_t int_value;
        float float_value;
        bool bool_value;
    };
} WbConfig;

_Static_assert(sizeof(WbConfig) == 32, "WbConfig must be 32 bytes.");
