#pragma once

typedef signed char s8;
typedef short s16;
typedef int s32;
typedef long long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef u16 f16;
typedef float f32;
typedef double f64;

#define S8_MIN (-127i8 - 1)
#define S16_MIN (-32767i16 - 1)
#define S32_MIN (-2147483647i32 - 1)
#define S64_MIN (-9223372036854775807i64 - 1)
#define S8_MAX 127i8
#define S16_MAX 32767i16
#define S32_MAX 2147483647i32
#define S64_MAX 9223372036854775807i64
#define U8_MAX 0xffui8
#define U16_MAX 0xffffui16
#define U32_MAX 0xffffffffui32
#define U64_MAX 0xffffffffffffffffui64

#ifndef bool
#define bool _Bool
#define false 0
#define true 1
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#define WB_HANDLE(size, name) \
	typedef struct {          \
		uint##size##_t id;    \
	} name

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