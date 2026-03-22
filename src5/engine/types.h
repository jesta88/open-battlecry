#pragma once

#if WK_PLATFORM_WINDOWS
typedef signed __int8 s8;
typedef unsigned __int8 u8;
typedef signed __int16 s16;
typedef unsigned __int16 u16;
typedef signed __int32 s32;
typedef unsigned __int32 u32;
typedef signed __int64 s64;
typedef unsigned __int64 u64;
#else
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
#endif

_Static_assert(sizeof(s8) == 1, "Invalid size for s8");
_Static_assert(sizeof(u8) == 1, "Invalid size for u8");
_Static_assert(sizeof(s16) == 2, "Invalid size for s16");
_Static_assert(sizeof(u16) == 2, "Invalid size for u16");
_Static_assert(sizeof(s32) == 4, "Invalid size for s32");
_Static_assert(sizeof(u32) == 4, "Invalid size for u32");
_Static_assert(sizeof(s64) == 8, "Invalid size for s64");
_Static_assert(sizeof(u64) == 8, "Invalid size for u64");

typedef u64 usize;
typedef u64 uptr;

typedef _Bool bool;
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
