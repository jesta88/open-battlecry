#pragma once 

#include <warkit/system.h>

struct wk_system
{
    uint64 tick_start;
    uint32 tick_numerator_ns;
    uint32 tick_denominator_ns;
    uint32 tick_numerator_ms;
    uint32 tick_denominator_ms;
};