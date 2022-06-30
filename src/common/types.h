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

#define S8_MIN (-127 - 1)
#define S16_MIN (-32767 - 1)
#define S32_MIN (-2147483647 - 1)
#define S64_MIN (-9223372036854775807LL - 1)
#define S8_MAX 127
#define S16_MAX 32767
#define S32_MAX 2147483647
#define S64_MAX 9223372036854775807LL
#define U8_MAX 255
#define U16_MAX 65535
#define U32_MAX 0xffffffffU
#define U64_MAX 0xffffffffffffffffULL

#ifndef bool
#define bool _Bool
#define false 0
#define true 1
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif