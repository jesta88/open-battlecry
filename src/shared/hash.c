#include "hash.h"
#define XXH_INLINE_ALL
#include <xxhash.h>
#include <string.h>

uint32_t wbHashString32(const char* input)
{
    return XXH32(input, strlen(input), 0);
}
