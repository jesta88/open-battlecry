#pragma once

#include "types.h"

typedef struct WbRandom
{
    uint64_t state[2];
} WbRandom;

void wbRandomInit(WbRandom* random, uint64_t seed);
uint32_t wbRandomUint(WbRandom* random);
float wbRandomFloat(WbRandom* random);

static inline int32_t wbRandomRangeInt(WbRandom* random, int32_t min, int32_t max)
{
    const uint32_t range = (uint32_t)(max - min) + 1;
    return min + (int32_t)(wbRandomUint(random) % range);
}

static inline float wbRandomRangeFloat(WbRandom* random, float min, float max)
{
    const float f = wbRandomFloat(random);
    return min + f * (max - min);
}
