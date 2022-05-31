#pragma once

#include "types.h"
#include <string.h>
#include <ctype.h>

static inline void wb_str_copy(char* dst, const char* src, size_t length)
{
	const uint32_t* last = (uint32_t*)_memccpy(dst, src, 0, length);
	if (!last)
		*(dst + length) = 0;
}

static inline int wb_str_get_extension(const char* file_name, char* result)
{
	char* dot = strrchr(file_name, '.');
	if (!dot || dot == file_name)
		return 1;

	const char* extension = dot + 1;
	const size_t extension_length = strlen(extension);
	if (extension_length == 0 || extension[0] == '/' || extension[0] == '\\')
		return 1;

	wb_str_copy(result, extension, extension_length);
	for (int i = 0; result[i]; i++)
		result[i] = tolower(result[i]);
	return 0;
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

static inline int32_t wb_str_find_s(const char* string, uint32_t string_size, const char* search, uint32_t search_size)
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

static inline int32_t wb_str_find(const char* string, const char* search)
{
	uint32_t string_length = (uint32_t)strlen(string);
	uint32_t search_length = (uint32_t)strlen(search);
	return wb_str_find_s(string, string_length, search, search_length);
}

static inline void wb_str_replace(char* dst, const char* src, const char* search, const char* replace)
{
	uint32_t src_length = (uint32_t)strlen(src);
	uint32_t search_length = (uint32_t)strlen(search);

	int32_t find_position = wb_str_find_s(src, src_length, search, search_length);

	uint32_t end_start = find_position + search_length;
	uint32_t end_length = src_length - end_start;

	char end[256];
	wb_str_copy(end, src + end_start, end_length);

	wb_str_copy(dst, src, find_position);
	strcat(dst, replace);
	strcat(dst, end);
}