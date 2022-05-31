#pragma once

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

#ifndef INT32_MAX
#define INT8_MIN         (-127i8 - 1)
#define INT16_MIN        (-32767i16 - 1)
#define INT32_MIN        (-2147483647i32 - 1)
#define INT64_MIN        (-9223372036854775807i64 - 1)
#define INT8_MAX         127i8
#define INT16_MAX        32767i16
#define INT32_MAX        2147483647i32
#define INT64_MAX        9223372036854775807i64
#define UINT8_MAX        0xffui8
#define UINT16_MAX       0xffffui16
#define UINT32_MAX       0xffffffffui32
#define UINT64_MAX       0xffffffffffffffffui64
#endif

#ifndef bool
#define bool  _Bool
#define false 0
#define true  1
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef struct
{
    float x, y;
} wb_float2;

typedef struct
{
    float x, y, z;
} wb_float3;

typedef struct
{
    float x, y, z, w;
} wb_float4;

typedef struct
{
    float xx, xy, xz;
    float yx, yy, yz;
} wb_float2x3;

typedef struct
{
    float xx, xy, xz, xw;
    float yx, yy, yz, yw;
    float zx, zy, zz, zw;
    float wx, wy, wz, ww;
} wb_float4x4;

typedef struct
{
    int x, y;
} wb_int2;

typedef struct
{
    int x, y, w, h;
} wb_rect;