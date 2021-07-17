#pragma once

#include <stdint.h>
#include <string.h>

static inline void string_copy(char* dst, const char* src, uint32_t length)
{
    const uint32_t* last = (uint32_t*) _memccpy(dst, src, 0, length);
    if (!last)
        *(dst + length) = 0;
}
