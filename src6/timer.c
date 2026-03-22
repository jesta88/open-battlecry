#include "timer.h"
#include "engine_private.h"
#include "log.h"

#ifdef _WIN32
typedef int BOOL;
typedef unsigned long DWORD;
typedef long LONG;
typedef long long LONGLONG;
typedef union {
    struct
    {
        DWORD LowPart;
        LONG HighPart;
    };
    LONGLONG QuadPart;
} LARGE_INTEGER;
BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);
BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);
#endif

void timer_init(void)
{
    bool result;
#ifdef _WIN32
    LARGE_INTEGER frequency_large;
    result = QueryPerformanceFrequency(&frequency_large);
    if (!result)
    {
        log_error("Could not query timer frequency.");
        return;
    }
    engine.timer.frequency = frequency_large.QuadPart;
#endif
}

uint64_t timer_ticks(void)
{
#ifdef _WIN32
    LARGE_INTEGER ticks_large;
    QueryPerformanceCounter(&ticks_large);
    ticks_large.QuadPart *= 1000;
    ticks_large.QuadPart /= engine.timer.frequency;
    return (uint64_t)ticks_large.QuadPart;
#endif
}
