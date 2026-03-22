#pragma once

#include "types.h"

#define WK_MS_PER_SECOND   1000
#define WK_US_PER_SECOND   1000000
#define WK_NS_PER_SECOND   1000000000LL
#define WK_NS_PER_MS       1000000
#define WK_NS_PER_US       1000
#define WK_MS_TO_NS(ms)    (((u64)(ms)) * WK_NS_PER_MS)
#define WK_NS_TO_MS(ns)    ((ns) / WK_NS_PER_MS)
#define WK_US_TO_NS(us)    (((u64)(us)) * WK_NS_PER_US)
#define WK_NS_TO_US(ns)    ((ns) / WK_NS_PER_US)

u64 wk_get_tick(void);
u64 wk_get_ticks_ms(void);
u64 wk_get_ticks_ns(void);

void wk_delay_ms(u64 ms);
void wk_delay_ns(u64 ns);