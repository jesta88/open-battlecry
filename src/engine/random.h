#pragma once

#include "types.h"

typedef struct random_t
{
    uint64_t state[2];
} WbRng;

void wb_rng_init(WbRng* rng, uint64_t seed);
uint32_t wb_rng_uint(WbRng* rng);
float wb_rng_float(WbRng* rng);

static inline int32_t random_range_int(WbRng* rng, int32_t min, int32_t max)
{
    const uint32_t range = (uint32_t)(max - min) + 1;
	return min + (int32_t)(wb_rng_uint(rng) % range);
}

static inline float random_range_float(WbRng* rng, float min, float max)
{
	const float f = wb_rng_float(rng);
    return min + f * (max - min);
}
