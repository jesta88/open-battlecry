#include "common.h"
#include <intrin.h>
#include <stdio.h>
#include <string.h>

#if WK_PLATFORM_WINDOWS
static void* g_hinstance;
static void* g_hwnd;
#endif

static char g_cpu_name[128];
static uint32 g_core_count;

static void print_cpu_info(void);

int wk_init_engine(const wk_engine_desc* engine_desc)
{
    // No buffering for std streams
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // Enable DAZ and FTZ 
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    // Init engine modules
    wk_init_file_system();
    wk_init_debug();
    wk_init_os();
    wk_init_threads();
    wk_init_time();

    // CPU info
    print_cpu_info();

    return 0;
}

void wk_quit_engine(void)
{
    wk_quit_file_system();
    wk_quit_threads();
    wk_quit_time();
    wk_quit_debug();
}

#if WK_PLATFORM_WINDOWS
void wk_set_win32_hinstance(void* hinstance)
{
    g_hinstance = hinstance;
}

void* wk_get_win32_hinstance(void)
{
    return g_hinstance;
}
#endif

static void print_cpu_info(void)
{
    int cpu_info[4] = { 0 };
    __cpuid(cpu_info, 1);
    const char* sse = "SSE2";
    const char* avx = "N/A";
    uint32 features = (int)cpu_info[2];
    if (features & (1 << 9))
        sse = "SSE3";
    if (features & (1 << 19))
        sse = "SSE41";
    if (features & (1 << 20))
        sse = "SSE42";
    if (features & (1 << 28))
        avx = "AVX";
    const char* popcnt = (features & (1 << 23)) ? " popcnt" : "";

    __cpuid(cpu_info, 7);
    features = (int)cpu_info[1];
    if (features & (1 << 5))
        avx = "AVX2";

    __cpuid(cpu_info, (int)0x80000000);
    const uint32 extended_ids = cpu_info[0];
    char cpu_brand[64] = { 0 };
    for (uint32 i = 0x80000000; i <= extended_ids; ++i)
    {
        __cpuid(cpu_info, i);
        if (i == 0x80000002)
            memcpy(cpu_brand, cpu_info, sizeof(cpu_info));
        else if (i == 0x80000003)
            memcpy(cpu_brand + 16, cpu_info, sizeof(cpu_info));
        else if (i == 0x80000004)
            memcpy(cpu_brand + 32, cpu_info, sizeof(cpu_info));
    }
    snprintf(g_cpu_name, sizeof(g_cpu_name), "<%.64s> %s %s%s", cpu_brand, sse, avx, popcnt);
}