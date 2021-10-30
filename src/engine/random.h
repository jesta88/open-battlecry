#pragma once

#include <stdint.h>

typedef struct random_t
{
    uint64_t state[2];
} random_t;

void random_init(random_t* random, uint64_t seed);
uint32_t random_uint(random_t* random);
float random_float(random_t* random);

static inline int32_t random_range_int(random_t* random, int32_t min, int32_t max)
{
    const uint32_t range = (uint32_t)(max - min) + 1;
    return min + (int32_t)(random_uint(random) % range);
}

static inline float random_range_float(random_t* random, float min, float max)
{
    const float f = random_float(random);
    return min + f * (max - min);
}
