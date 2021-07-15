#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct WbBitset8
{
    uint8_t bits;
} WbBitset8;

typedef struct WbBitset256
{
    uint64_t bits[4];
} WbBitset256;

static inline bool wbHasFlag8(uint8_t value, uint8_t flag)
{
    return (value & flag) != 0;
}

static inline bool wbHasFlag16(uint16_t value, uint16_t flag)
{
    return (value & flag) != 0;
}

static inline bool wbHasFlag32(uint32_t value, uint32_t flag)
{
    return (value & flag) != 0;
}

static inline uint32_t wbFastMod(uint32_t number, uint32_t power_of_two)
{
    return (number & (power_of_two - 1));
}

static inline void wbSetBit8(WbBitset8* bitset8, size_t position)
{
    bitset8->bits |= 1 << position;
}

static inline void wbClearBit8(WbBitset8* bitset8, size_t position)
{
    bitset8->bits &= ~(1 << position);
}

static inline bool wbIsBitSet8(WbBitset8 bitset8, size_t position)
{
    return (bitset8.bits & (1 << position));
}

static inline void wbToggleBit8(WbBitset8* bitset8, size_t position)
{
    bitset8->bits ^= 1 << position;
}

static inline void wbSetBit256(WbBitset256* bitset256, size_t position)
{
    bitset256->bits[position >> 2] |= 1ull << position;
}

static inline void wbClearBit256(WbBitset256* bitset256, size_t position)
{
    bitset256->bits[position >> 2] &= ~(1ull << position);
}

static inline bool wbIsBitSet256(const WbBitset256* bitset256, size_t position)
{
    return (bitset256->bits[position >> 2] & (1ull << position));
}

static inline void wbToggleBit256(WbBitset256* bitset256, size_t position)
{
    bitset256->bits[position >> 2] ^= 1ull << position;
}
