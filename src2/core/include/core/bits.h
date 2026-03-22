#pragma once

#include "../../common.h"

// Bit manipulation utilities - only included where needed
// This keeps common.h lightweight while providing optimized bit operations

#if defined(_MSC_VER)
    // Declare intrinsics without including the heavy intrin.h header
    #pragma intrinsic(_BitScanReverse64)
    #pragma intrinsic(_BitScanForward64)
    unsigned char _BitScanReverse64(unsigned long* _Index, unsigned __int64 _Mask);
    unsigned char _BitScanForward64(unsigned long* _Index, unsigned __int64 _Mask);
    
    static inline int war_clz64(u64 value) {
        unsigned long index;
        return _BitScanReverse64(&index, value) ? (63 - (int)index) : 64;
    }
    
    static inline int war_ctz64(u64 value) {
        unsigned long index;
        return _BitScanForward64(&index, value) ? (int)index : 64;
    }
    
#elif defined(__GNUC__) || defined(__clang__)
    
    static inline int war_clz64(u64 value) {
        return value ? __builtin_clzll(value) : 64;
    }
    
    static inline int war_ctz64(u64 value) {
        return value ? __builtin_ctzll(value) : 64;
    }
    
#else
    // Fallback implementation (portable but slower)
    static inline int war_clz64(u64 value) {
        if (value == 0) return 64;
        int count = 0;
        if (!(value >> 32)) { count += 32; value <<= 32; }
        if (!(value >> 48)) { count += 16; value <<= 16; }
        if (!(value >> 56)) { count += 8;  value <<= 8;  }
        if (!(value >> 60)) { count += 4;  value <<= 4;  }
        if (!(value >> 62)) { count += 2;  value <<= 2;  }
        if (!(value >> 63)) { count += 1; }
        return count;
    }
    
    static inline int war_ctz64(u64 value) {
        if (value == 0) return 64;
        int count = 0;
        if (!(value & 0xFFFFFFFF)) { count += 32; value >>= 32; }
        if (!(value & 0xFFFF)) { count += 16; value >>= 16; }
        if (!(value & 0xFF)) { count += 8; value >>= 8; }
        if (!(value & 0xF)) { count += 4; value >>= 4; }
        if (!(value & 0x3)) { count += 2; value >>= 2; }
        if (!(value & 0x1)) { count += 1; }
        return count;
    }
#endif

// Convenient macros for bit operations
#define WAR_COUNT_LEADING_ZEROS(x) war_clz64(x)
#define WAR_COUNT_TRAILING_ZEROS(x) war_ctz64(x)
