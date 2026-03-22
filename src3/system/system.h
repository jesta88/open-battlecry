#pragma once

#include "common.h"

#if WK_DEBUG
	#define WK_FATAL_PREFIX
	#define WK_FATAL_SUFFIX
#elif defined(WK_COMPILER_MSVC)
	#define WK_FATAL_PREFIX WK_NO_RETURN
	#define WK_FATAL_SUFFIX
#else
	#define WK_FATAL_PREFIX
	#define WK_FATAL_SUFFIX WK_NO_RETURN
#endif

#define WK_MS_PER_SECOND 1000
#define WK_US_PER_SECOND 1000000
#define WK_NS_PER_SECOND 1000000000LL
#define WK_NS_PER_MS 1000000
#define WK_NS_PER_US 1000
#define WK_MS_TO_NS(ms) (((uint64)(ms)) * WK_NS_PER_MS)
#define WK_NS_TO_MS(ns) ((ns) / WK_NS_PER_MS)
#define WK_US_TO_NS(us) (((uint64)(us)) * WK_NS_PER_US)
#define WK_NS_TO_US(ns) ((ns) / WK_NS_PER_US)

typedef struct wk_system wk_system;

typedef struct wk_cpu_info
{
	bool has_sse2;
	bool has_sse3;
	bool has_sse4;
	bool has_avx;
	bool has_avx2;
	bool has_popcnt;
	char cpu_name[64];
} wk_cpu_info;

WK_API int wk_init_cpu_info(wk_cpu_info* cpu_info);

WK_API void* wk_load_library(const char* name);
WK_API void wk_unload_library(void* library);
WK_API void* wk_load_function(void* library, const char* name);

WK_API uint64 wk_get_tick(void);
WK_API uint64 wk_get_ticks_ms(void);
WK_API uint64 wk_get_ticks_ns(void);

WK_API void wk_sleep_ms(uint64 ms);
WK_API void wk_sleep_ns(uint64 ns);

WK_API WK_FATAL_PREFIX void wk_fatal(const char* message, ...) WK_FATAL_SUFFIX WK_FORMAT_PRINTF(1, 2);
