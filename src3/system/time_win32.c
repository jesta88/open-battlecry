#include "time_internal.h"
#include "windows/windows_misc.h"
#include "windows/windows_threads.h"
#include "warkit/system.h"

#define WK_TIME_PERIOD_MS 1

static uint64 tick_start;
static uint32 tick_numerator_ns;
static uint32 tick_denominator_ns;
static uint32 tick_numerator_ms;
static uint32 tick_denominator_ms;

static DWORD tls_index = TLS_OUT_OF_INDEXES;

static uint32 calculate_common_denominator(const uint32 a, const uint32 b)
{
    if (b == 0) {
        return a;
    }
    return calculate_common_denominator(b, (a % b));
}

void wk_init_time(void)
{
    timeBeginPeriod(WK_TIME_PERIOD_MS);

    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    const uint64 tick_frequency = li.QuadPart;

    uint32 common_denominator = calculate_common_denominator(WK_NS_PER_SECOND, (uint32)tick_frequency);
    tick_numerator_ns = (WK_NS_PER_SECOND / common_denominator);
    tick_denominator_ns = (uint32)(tick_frequency / common_denominator);

    common_denominator = calculate_common_denominator(WK_MS_PER_SECOND, (uint32)tick_frequency);
    tick_numerator_ms = (WK_MS_PER_SECOND / common_denominator);
    tick_denominator_ms = (uint32)(tick_frequency / common_denominator);

    QueryPerformanceCounter(&li);
    tick_start = li.QuadPart;
    if (!tick_start) {
        --tick_start;
    }

    // Main thread TLS
    wk_init_time_tls();
}

void wk_quit_time(void)
{
    if (tls_index != TLS_OUT_OF_INDEXES) {
        const HANDLE timer = TlsGetValue(tls_index);
        if (timer) {
            CloseHandle(timer);
        }
        TlsFree(tls_index);
    }
    timeEndPeriod(WK_TIME_PERIOD_MS);
    tick_start = 0;
}

void wk_init_time_tls(void)
{
    if (tls_index == TLS_OUT_OF_INDEXES) {
        tls_index = TlsAlloc();
    }

    if (tls_index != TLS_OUT_OF_INDEXES) {
        const HANDLE timer = CreateWaitableTimerExA(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
                                                    TIMER_ALL_ACCESS);
        if (timer) {
            if (!TlsSetValue(tls_index, timer)) {
                wk_log_error("TlsSetValue failed");
            }
        }
    }
}

uint64 wk_get_tick(void)
{
    LARGE_INTEGER tick;
    QueryPerformanceCounter(&tick);
    return tick.QuadPart;
}

uint64 wk_get_ticks_ms(void)
{
    LARGE_INTEGER tick;
    QueryPerformanceCounter(&tick);

    const uint64 ticks = tick.QuadPart - tick_start;
    return ticks * tick_numerator_ms / tick_denominator_ms;
}

uint64 wk_get_ticks_ns(void)
{
    LARGE_INTEGER tick;
    QueryPerformanceCounter(&tick);

    const uint64 ticks = tick.QuadPart - tick_start;
    return ticks * tick_numerator_ns / tick_denominator_ns;
}

void wk_delay_ms(const uint64 ms)
{
    wk_delay_ns(WK_NS_TO_MS(ms));
}

void wk_delay_ns(uint64 ns)
{
    if (tls_index != TLS_OUT_OF_INDEXES) {
        const HANDLE timer = TlsGetValue(tls_index);
        if (timer) {
            LARGE_INTEGER due_time;
            due_time.QuadPart = -((LONGLONG)ns / 100);
            if (SetWaitableTimerEx(timer, &due_time, 0, NULL, NULL, NULL, 0)) {
                WaitForSingleObject(timer, INFINITE);
            }
            return;
        }
    }

    // Fallback to Sleep
    const uint64 max_delay = 0xffffffffLL * WK_NS_PER_MS;
    if (ns > max_delay) {
        ns = max_delay;
    }
    Sleep((DWORD)WK_NS_TO_MS(ns));
}
