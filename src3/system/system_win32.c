#include "system_internal.h"
#include "warkit/math.h"
#include "windows/windows_dbghelp.h"
#include "windows/windows_io.h"
#include "windows/windows_misc.h"
#include "windows/windows_threads.h"
#include "windows/windows_window.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define WK_TIME_PERIOD_MS 1

WK_API int NvOptimusEnablement = 0x00000001;
WK_API int AmdPowerXpressRequestHighPerformance = 0x00000001;

static wk_system system;
static DWORD     tls_index = TLS_OUT_OF_INDEXES;
static bool      has_console;
static HANDLE    stdout_handle;

int WINAPI WinMain(void* instance, void* prev_instance, char** cmd_line, int show_cmd)
{
    (void)prev_instance;
    (void)cmd_line;
    (void)show_cmd;

    // Console
    {
        has_console = AttachConsole(ATTACH_PARENT_PROCESS);
#ifdef WK_DEBUG
        if (!has_console)
            has_console = AllocConsole();
#endif

        if (has_console) {
            FILE* stream = NULL;
            freopen_s(&stream, "CONOUT$", "w", stdout);
            freopen_s(&stream, "CONOUT$", "w", stderr);
            SetConsoleOutputCP(CP_UTF8);
            stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
            assert(stdout_handle != INVALID_HANDLE_VALUE);
        }
    }

    // Timer
    {
        timeBeginPeriod(WK_TIME_PERIOD_MS);

        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        const uint64 tick_frequency = li.QuadPart;

        uint32 common_denominator = wk_common_denominator(WK_NS_PER_SECOND, (uint32)tick_frequency);
        system.tick_numerator_ns = (WK_NS_PER_SECOND / common_denominator);
        system.tick_denominator_ns = (uint32)(tick_frequency / common_denominator);

        common_denominator = wk_common_denominator(WK_MS_PER_SECOND, (uint32)tick_frequency);
        system.tick_numerator_ms = (WK_MS_PER_SECOND / common_denominator);
        system.tick_denominator_ms = (uint32)(tick_frequency / common_denominator);

        QueryPerformanceCounter(&li);
        system.tick_start = li.QuadPart;
        if (!system.tick_start)
            --system.tick_start;
    }

    // Paths
    {

    }

    wk_main();

    return 0;
}

void wk_init_time_tls(void)
{
    if (tls_index == TLS_OUT_OF_INDEXES)
        tls_index = TlsAlloc();

    if (tls_index != TLS_OUT_OF_INDEXES) {
        const HANDLE timer = CreateWaitableTimerExA(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
                                                    TIMER_ALL_ACCESS);
        assert(timer);

        if (!TlsSetValue(tls_index, timer))
            wk_fatal("TlsSetValue");
    }
}

void wk_fatal(const char* message, ...)
{
#ifdef WK_DEBUG
    OutputDebugStringA(message);
#endif

    DWORD console_mode;
    if (!GetConsoleMode(stdout_handle, &console_mode))
        wk_fatal("GetConsoleMode");

    if (console_mode == ENABLE_ECHO_INPUT) {
#ifdef WK_DEBUG
        if (IsDebuggerPresent()) {
            OutputDebugStringA("Stack trace:\n");
            fprintf(stderr, "%s\n", "Stack trace:");
            DebugBreak();
            exit(1);
        } else
#endif
        {
            MessageBoxA(NULL, message, "System Error", MB_OK | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST | MB_ICONERROR);
            abort();
        }
    } else {
#ifdef WK_DEBUG
        OutputDebugStringA("Stack trace:\n");
        fprintf(stderr, "%s\n", "Stack trace:");
        if (IsDebuggerPresent())
            DebugBreak();
#else
        exit(1);
#endif
    }
}

int wk_init_cpu_info(wk_cpu_info* cpu_info)
{
	return 0;
}