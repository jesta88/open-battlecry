#pragma once

#include "std.h"

typedef struct random_t
{
    u64 state[2];
} WbRng;

void wb_rng_init(WbRng* rng, u64 seed);
u32 wb_rng_uint(WbRng* rng);
float wb_rng_float(WbRng* rng);

static inline s32 random_range_int(WbRng* rng, s32 min, s32 max)
{
    const u32 range = (u32)(max - min) + 1;
	return min + (s32)(wb_rng_uint(rng) % range);
}

static inline float random_range_float(WbRng* rng, float min, float max)
{
	const float f = wb_rng_float(rng);
    return min + f * (max - min);
}
