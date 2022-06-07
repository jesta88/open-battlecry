#pragma once

#include "types.h"

typedef struct
{
    u8 bits;
} wb_bitset8;

typedef struct
{
    u64 bits[4];
} wb_bitset256;

static inline bool wb_is_flag_set(u32 value, u32 flag)
{
    return (value & flag) != 0;
}

static inline u32 wb_mod_pow2(u32 number, u32 power_of_two)
{
    return (number & (power_of_two - 1));
}

static inline void wb_bits8_set(wb_bitset8* bitset8, u64 position)
{
    bitset8->bits |= 1 << position;
}

static inline void wb_bits8_clear(wb_bitset8* bitset8, u64 position)
{
    bitset8->bits &= ~(1 << position);
}

static inline bool wb_bits8_is_set(wb_bitset8 bitset8, u64 position)
{
    return (bitset8.bits & (1 << position));
}

static inline void wb_bits8_toggle(wb_bitset8* bitset8, u64 position)
{
    bitset8->bits ^= 1 << position;
}

static inline void wb_bits256_set(wb_bitset256* bitset256, u64 position)
{
    bitset256->bits[position >> 2] |= 1ull << position;
}

static inline void wb_bits256_clear(wb_bitset256* bitset256, u64 position)
{
    bitset256->bits[position >> 2] &= ~(1ull << position);
}

static inline bool wb_bits256_is_set(const wb_bitset256* bitset256, u64 position)
{
    return (bitset256->bits[position >> 2] & (1ull << position));
}

static inline void wb_bits256_toggle(wb_bitset256* bitset256, u64 position)
{
    bitset256->bits[position >> 2] ^= 1ull << position;
}
