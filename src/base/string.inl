#pragma once

#include <stdint.h>
#include <string.h>

static inline void string_copy(char* dst, const char* src, uint32_t length)
{
    const uint32_t* last = (uint32_t*) _memccpy(dst, src, 0, length);
    if (!last)
        *(dst + length) = 0;
}

static inline int32_t string_find_length(const char* string, uint32_t string_length, const char* search, uint32_t search_length)
{
    int i = 0;
    int j = 0;
    int find_start = 0;
    while (search_length - j <= string_length - i)
    {
        if (string[i] == search[j])
        {
            j++;
        }
        else
        {
            find_start = i + 1;
            j = 0;
        }

        if (j == search_length)
            return find_start;

        ++i;
    }

    return -1;
}

static inline int32_t string_find(const char* string, const char* search)
{
    uint32_t string_length = (uint32_t) strlen(string);
    uint32_t search_length = (uint32_t) strlen(search);
    return string_find_length(string, string_length, search, search_length);
}

static inline void string_replace(char* dst, const char* src, const char* search, const char* replace)
{
    uint32_t src_length = (uint32_t) strlen(src);
    uint32_t search_length = (uint32_t) strlen(search);

    int32_t find_position = string_find_length(src, src_length, search, search_length);

    uint32_t end_start = find_position + search_length;
    uint32_t end_length = src_length - end_start;

    char end[256];
    string_copy(end, src + end_start, end_length);

    string_copy(dst, src, find_position);
    strcat(dst, replace);
    strcat(dst, end);
}