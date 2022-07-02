#pragma once

#include "types.h"
#include <string.h>

static inline u64 murmur_hash_64(const void* key, u32 length, u64 seed)
{
    const u64 m = 0xc6a4a7935bd1e995ULL;
    const int r = 47;

    u64 h = seed ^ (length * m);

    const u64* data = (const u64*) key;
    const u64* end = data ? data + (length / 8) : data;

    while (data != end)
    {
        u64 k;
        memcpy(&k, data++, sizeof(k));

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const u8* data2 = (const u8*) data;
    if (data2 == NULL)
    {
        return 0;
    }

    switch (length & 7) {
        case 7:
            h ^= (u64) (data2[6]) << 48;
        case 6:
            h ^= (u64) (data2[5]) << 40;
        case 5:
            h ^= (u64) (data2[4]) << 32;
        case 4:
            h ^= (u64) (data2[3]) << 24;
        case 3:
            h ^= (u64) (data2[2]) << 16;
        case 2:
            h ^= (u64) (data2[1]) << 8;
        case 1:
            h ^= (u64) (data2[0]);
            h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}