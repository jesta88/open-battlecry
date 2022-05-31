#pragma once

#include "types.h"

typedef struct
{
    uint8_t bits;
} wb_bitset8;

typedef struct
{
    uint64_t bits[4];
} wb_bitset256;

static inline bool wb_is_flag_set(uint32_t value, uint32_t flag)
{
    return (value & flag) != 0;
}

static inline uint32_t wb_mod_pow2(uint32_t number, uint32_t power_of_two)
{
    return (number & (power_of_two - 1));
}

static inline void wb_bits8_set(wb_bitset8* bitset8, uint64_t position)
{
    bitset8->bits |= 1 << position;
}

static inline void wb_bits8_clear(wb_bitset8* bitset8, uint64_t position)
{
    bitset8->bits &= ~(1 << position);
}

static inline bool wb_bits8_is_set(wb_bitset8 bitset8, uint64_t position)
{
    return (bitset8.bits & (1 << position));
}

static inline void wb_bits8_toggle(wb_bitset8* bitset8, uint64_t position)
{
    bitset8->bits ^= 1 << position;
}

static inline void wb_bits256_set(wb_bitset256* bitset256, uint64_t position)
{
    bitset256->bits[position >> 2] |= 1ull << position;
}

static inline void wb_bits256_clear(wb_bitset256* bitset256, uint64_t position)
{
    bitset256->bits[position >> 2] &= ~(1ull << position);
}

static inline bool wb_bits256_is_set(const wb_bitset256* bitset256, uint64_t position)
{
    return (bitset256->bits[position >> 2] & (1ull << position));
}

static inline void wb_bits256_toggle(wb_bitset256* bitset256, uint64_t position)
{
    bitset256->bits[position >> 2] ^= 1ull << position;
}
