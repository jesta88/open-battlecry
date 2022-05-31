#include "random.h"

// TODO: Compare perf with: https://github.com/lemire/SIMDxorshift

static inline float rng_float_normalized(uint32_t value)
{
    uint32_t exponent = 127;
    uint32_t mantissa = value >> 9;
    uint32_t result = (exponent << 23) | mantissa;
    float fresult = *(float*)(&result);
    return fresult - 1.0f;
}

static inline uint64_t rng_avalanche64(uint64_t h)
{
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33;
    return h;
}

void wb_rng_init(WbRng* random, uint64_t seed)
{
    uint64_t value = ((seed) << 1ull) | 1ull; // Make it odd
	value = rng_avalanche64(value);
    random->state[0] = 0ull;
    random->state[1] = (value << 1ull) | 1ull;
    wb_rng_uint(random);
	random->state[0] += rng_avalanche64(value);
    wb_rng_uint(random);
}

uint32_t wb_rng_uint(WbRng* rng)
{
	uint64_t old_state = rng->state[0];
	rng->state[0] = old_state * 0x5851f42d4c957f2dull + rng->state[1];
    uint32_t xorshifted = (uint32_t)(((old_state >> 18ull) ^ old_state) >> 27ull);
    uint32_t rot = (uint32_t)(old_state >> 59ull);
    return (xorshifted >> rot) | (xorshifted << ((-(int)rot) & 31));
}

float wb_rng_float(WbRng* rng)
{
	return rng_float_normalized(wb_rng_uint(rng));
}
