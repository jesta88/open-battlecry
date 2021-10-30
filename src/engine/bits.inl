#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct bits8_t
{
    uint8_t bits;
} bits8_t;

typedef struct bits256_t
{
    uint64_t bits[4];
} bits256_t;

static inline bool is_flag_set(uint32_t value, uint32_t flag)
{
    return (value & flag) != 0;
}

static inline uint32_t mod_pow2(uint32_t number, uint32_t power_of_two)
{
    return (number & (power_of_two - 1));
}

static inline void bits8_set(bits8_t* bitset8, uint64_t position)
{
    bitset8->bits |= 1 << position;
}

static inline void bits8_clear(bits8_t* bitset8, uint64_t position)
{
    bitset8->bits &= ~(1 << position);
}

static inline bool bits8_is_set(bits8_t bitset8, uint64_t position)
{
    return (bitset8.bits & (1 << position));
}

static inline void bits8_toggle(bits8_t* bitset8, uint64_t position)
{
    bitset8->bits ^= 1 << position;
}

static inline void bits256_set(bits256_t* bitset256, uint64_t position)
{
    bitset256->bits[position >> 2] |= 1ull << position;
}

static inline void bits256_clear(bits256_t* bitset256, uint64_t position)
{
    bitset256->bits[position >> 2] &= ~(1ull << position);
}

static inline bool bits256_is_set(const bits256_t* bitset256, uint64_t position)
{
    return (bitset256->bits[position >> 2] & (1ull << position));
}

static inline void bits256_toggle(bits256_t* bitset256, uint64_t position)
{
    bitset256->bits[position >> 2] ^= 1ull << position;
}
