#include "random.h"

// TODO: Compare perf with: https://github.com/lemire/SIMDxorshift

static inline float rng_float_normalized(u32 value)
{
    u32 exponent = 127;
    u32 mantissa = value >> 9;
    u32 result = (exponent << 23) | mantissa;
    float fresult = *(float*)(&result);
    return fresult - 1.0f;
}

static inline u64 rng_avalanche64(u64 h)
{
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33;
    return h;
}

void wb_rng_init(WbRng* random, u64 seed)
{
    u64 value = ((seed) << 1ull) | 1ull; // Make it odd
	value = rng_avalanche64(value);
    random->state[0] = 0ull;
    random->state[1] = (value << 1ull) | 1ull;
    wb_rng_uint(random);
	random->state[0] += rng_avalanche64(value);
    wb_rng_uint(random);
}

u32 wb_rng_uint(WbRng* rng)
{
	u64 old_state = rng->state[0];
	rng->state[0] = old_state * 0x5851f42d4c957f2dull + rng->state[1];
    u32 xorshifted = (u32)(((old_state >> 18ull) ^ old_state) >> 27ull);
    u32 rot = (u32)(old_state >> 59ull);
    return (xorshifted >> rot) | (xorshifted << ((-(int)rot) & 31));
}

float wb_rng_float(WbRng* rng)
{
	return rng_float_normalized(wb_rng_uint(rng));
}
