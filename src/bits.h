#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct WbBitset8
{
    uint8_t bits;
} WbBitset8;

typedef struct WbBitset256
{
    uint64_t bits[4];
} WbBitset256;

static inline uint32_t wbFastMod(uint32_t number, uint32_t power_of_two)
{
    return (number & (power_of_two - 1));
}

static inline void wbSetBit8(WbBitset8* bitset8, uint8_t position)
{
    bitset8->bits |= 1 << position;
}

static inline void wbClearBit8(WbBitset8* bitset8, uint8_t position)
{
    bitset8->bits &= ~(1 << position);
}

static inline bool wbCheckBit8(const WbBitset8* bitset8, uint8_t position)
{
    return (bitset8->bits & (1 << position));
}

static inline void wbToggleBit8(WbBitset8* bitset8, uint8_t position)
{
    bitset8->bits ^= 1 << position;
}

static inline void wbSetBit256(WbBitset256* bitset256, uint8_t position)
{
    bitset256->bits[position >> 8] |= 1 << position;
}

static inline void wbClearBit256(WbBitset256* bitset256, uint8_t position)
{
    bitset256->bits[position >> 8] &= ~(1 << position);
}

static inline bool wbCheckBit256(const WbBitset256* bitset256, uint8_t position)
{
    return (bitset256->bits[position >> 8] & (1 << position));
}

static inline void wbToggleBit256(WbBitset256* bitset256, uint8_t position)
{
    bitset256->bits[position >> 8] ^= 1 << position;
}