#pragma once

#include "std.h"
#include <strings.h>
#include <ctype.h>

static inline void wb_string_copy(char* dst, const char* src, size_t length)
{
	const u32* last = (u32*)_memccpy(dst, src, 0, length);
	if (!last)
		*(dst + length) = 0;
}

static inline void wb_str_to_snake_case(char* out, size_t out_size, const char* in)
{
	const char* inp = in;
	size_t n = 0;
	while (n < out_size - 1 && *inp)
	{
		if (*inp >= 'A' && *inp <= 'Z')
		{
			if (n > out_size - 3)
			{
				out[n++] = 0;
				return;
			}
			if (n > 0)
			{
				out[n++] = '_';
			}
			out[n++] = *inp + ('a' - 'A');
		}
		else
		{
			out[n++] = *inp;
		}
		++inp;
	}
	out[n++] = 0;
}

static inline s32 wb_str_find_s(const char* string, u32 string_size, const char* search, u32 search_size)
{
	int i = 0;
	int j = 0;
	int find_start = 0;
	while (search_size - j <= string_size - i)
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

		if (j == search_size)
			return find_start;

		++i;
	}

	return -1;
}

static inline s32 wb_str_find(const char* string, const char* search)
{
	u32 string_length = (u32)strlen(string);
	u32 search_length = (u32)strlen(search);
	return wb_str_find_s(string, string_length, search, search_length);
}

static inline void wb_str_replace(char* dst, const char* src, const char* search, const char* replace)
{
	u32 src_length = (u32)strlen(src);
	u32 search_length = (u32)strlen(search);

	s32 find_position = wb_str_find_s(src, src_length, search, search_length);

	u32 end_start = find_position + search_length;
	u32 end_length = src_length - end_start;

	char end[256];
	wb_string_copy(end, src + end_start, end_length);

	wb_string_copy(dst, src, find_position);
	strcat(dst, replace);
	strcat(dst, end);
}