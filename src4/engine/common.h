#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define WBC_PLATFORM_WINDOWS 1
#else
    #define WBC_PLATFORM_POSIX 1
#endif

#define WBC_CACHE_LINE      64
#define WBC_ALIGNAS(x)      _Alignas(x)
#define WBC_INLINE          static inline
#define WBC_ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifdef WBC_PLATFORM_WINDOWS
    #define WBC_NOINLINE        __declspec(noinline)
    #define WBC_THREAD_LOCAL    __declspec(thread)
#else
    #define WBC_NOINLINE        __attribute__((noinline))
    #define WBC_THREAD_LOCAL    _Thread_local
#endif

/* Debug break */
#ifdef WBC_PLATFORM_WINDOWS
    #define WBC_DEBUG_BREAK() __debugbreak()
#elif defined(__has_builtin) && __has_builtin(__builtin_debugtrap)
    #define WBC_DEBUG_BREAK() __builtin_debugtrap()
#elif defined(__GNUC__) || defined(__clang__)
    #define WBC_DEBUG_BREAK() __builtin_trap()
#else
    #include <signal.h>
    #define WBC_DEBUG_BREAK() raise(SIGTRAP)
#endif

#define WBC_ASSERT(cond) do { \
    if (!(cond)) { \
        WBC_DEBUG_BREAK(); \
    } \
    } while(0)

#ifdef NDEBUG
    #define WBC_DEBUG_ASSERT(cond) ((void)0)
#else
    #define WBC_DEBUG_ASSERT(cond) WBC_ASSERT(cond)
#endif
