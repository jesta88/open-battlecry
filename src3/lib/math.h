#pragma once

#include "common.h"

static WK_API uint32 wk_common_denominator(const uint32 a, const uint32 b)
{
    if (b == 0)
        return a;
    return wk_common_denominator(b, (a % b));
}