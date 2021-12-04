#pragma once

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

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

#define bool  _Bool
#define false 0
#define true  1

#define NULL ((void *)0)

typedef struct
{
    float x, y;
} vec2_t;

typedef struct
{
    float x, y, z;
} vec3_t;

typedef struct
{
    float x, y, z, w;
} vec4_t;

typedef struct
{
    float xx, xy, xz;
    float yx, yy, yz;
} mat23_t;

typedef struct
{
    float xx, xy, xz, xw;
    float yx, yy, yz, yw;
    float zx, zy, zz, zw;
    float wx, wy, wz, ww;
} mat44_t;

typedef struct
{
    int x, y;
} point_t;

typedef struct
{
    int x, y, w, h;
} rect_t;