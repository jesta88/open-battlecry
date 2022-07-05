#pragma once

#include "../common/types.h"

typedef struct wb_image
{
	char* name;
	u8* source_data;
	u8* compressed_data;
	u64 source_size;
	u64 compressed_size;
	u32 name_size;
	u32 width;
	u32 height;
	u32 vk_format;
	u32 _pad[2];
} wb_image;

_Static_assert(sizeof(wb_image) == 64, "wb_image should fit in a cache line");

void wb_image_load_bmp(const char* name, const u8* data, u32 size, wb_image* image);
void wb_image_load_rle(const char* name, const u8* data, wb_image* image);
void wb_image_free(wb_image* image);