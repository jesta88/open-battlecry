#pragma once

#ifndef NULL
#define NULL ((void*)0)
#endif

#define WK_SUCCESS 0

#define WK_MAX_PATH 256

#define WK_INVALID_ID_U64 18446744073709551615UL
#define WK_INVALID_ID_U32 4294967295U
#define WK_INVALID_ID_U16 65535U
#define WK_INVALID_ID_U8 255U

#define WK_GIBIBYTES(x) ((x) * 1024ULL * 1024ULL * 1024ULL)
#define WK_MEBIBYTES(x) ((x) * 1024ULL * 1024ULL)
#define WK_KIBIBYTES(x) ((x) * 1024ULL)

#define WK_GIGABYTES(x) ((x) * 1000ULL * 1000ULL * 1000ULL)
#define WK_MEGABYTES(x) ((x) * 1000ULL * 1000ULL)
#define WK_KILOBYTES(x) ((x) * 1000ULL)

#define WK_TICKRATE_240 0.0041666666666667f
#define WK_TICKRATE_144 0.0069444444444444f
#define WK_TICKRATE_120 0.0083333333333333f
#define WK_TICKRATE_60 0.0166666666666667f
#define WK_TICKRATE_30 0.0333333333333333f

#if (defined(__GNUC__) || defined(__clang__)) && !defined(__MACOSX__)
#define WK_PRINTF(x, y)  __attribute__((format(printf, x, y)))
#else
#define WK_PRINTF(x, y)
#endif

// Compiler defines
#if WK_ENGINE_DLL
#   if WK_ENGINE_EXPORT
#       define WK_API __declspec(dllexport)
#   else
#       define WK_API __declspec(dllimport)
#   endif
#else
#   define WK_API
#endif

#if defined(_MSC_VER)
#   define WK_INLINE __forceinline
#   define WK_NO_INLINE __declspec(noinline)
#elif defined(__clang__) || defined(__gcc__)
#   define WK_INLINE __attribute__((always_inline)) inline
#   define WK_NO_INLINE __attribute__((noinline))
#   ifdef WK_ENGINE_EXPORT
#       define WK_API __attribute__((visibility("default")))
#   else
#       define WK_API
#   endif
#endif

#define wk_countof(x) (sizeof(x) / sizeof((x)[0]))

#define wk_min(x, y) (x < y ? x : y)
#define wk_max(x, y) (x > y ? x : y)
#define wk_clamp(value, min, max) ((value <= min) ? min : (value >= max) ? max : value)