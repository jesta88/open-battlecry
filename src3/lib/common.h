#pragma once

#include <stdint.h>

// Shared library
#if defined(_MSC_VER) && defined(warkit_EXPORTS)
    #define WK_EXPORT __declspec(dllexport)
#elif defined(_MSC_VER) && defined(WK_DLL)
    #define WK_EXPORT __declspec(dllimport)
#elif defined(warkit_EXPORTS)
    #define WK_EXPORT __attribute__((visibility("default")))
#else
    #define WK_EXPORT
#endif

#define WK_API WK_EXPORT

// Misc
#ifdef WK_MSVC
    #define WK_INLINE    __forceinline
    #define WK_NO_INLINE __declspec(noinline)
    #define WK_NO_RETURN __declspec(noreturn)
#else
    #define WK_INLINE    __attribute__((always_inline)) inline
    #define WK_NO_INLINE __attribute__((noinline))
    #define WK_NO_RETURN __attribute__((noreturn))
#endif

// Printf
#if (defined(WK_GCC) || defined(WK_CLANG)) && !defined(WK_MACOS)
    #define WK_FORMAT_PRINTF(x, y) __attribute__((format(printf, x, y)))
#else
    #define WK_FORMAT_PRINTF(x, y)
#endif

// Constant macros
#define WK_SUCCESS                0

#define WK_MAX_PATH               256

#define WK_GIBIBYTES(x)           ((x) * 1024ULL * 1024ULL * 1024ULL)
#define WK_MEBIBYTES(x)           ((x) * 1024ULL * 1024ULL)
#define WK_KIBIBYTES(x)           ((x) * 1024ULL)

#define WK_GIGABYTES(x)           ((x) * 1000ULL * 1000ULL * 1000ULL)
#define WK_MEGABYTES(x)           ((x) * 1000ULL * 1000ULL)
#define WK_KILOBYTES(x)           ((x) * 1000ULL)

#define WK_TICKRATE_240           0.0041666666666667f
#define WK_TICKRATE_144           0.0069444444444444f
#define WK_TICKRATE_120           0.0083333333333333f
#define WK_TICKRATE_60            0.0166666666666667f
#define WK_TICKRATE_30            0.0333333333333333f

// Function macros
#define WK_countof(x)             (sizeof(x) / sizeof((x)[0]))

#define WK_min(x, y)              (x < y ? x : y)
#define WK_max(x, y)              (x > y ? x : y)
#define WK_clamp(value, min, max) ((value <= min) ? min : (value >= max) ? max : value)