#include "hash.h"
#include <xxhash.h>

uint32_t hash32(const char* input, uint64_t length, uint32_t seed)
{
    return (uint32_t) XXH64(input, length, seed);
}
