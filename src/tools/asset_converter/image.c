#include "image.h"

#define STBI_ONLY_BMP
#define STBI_NO_STDIO
#include "../../third_party/stb/stb_image.h"

#include <string.h>
#include <assert.h>

#define pack_rgba(r, g, b, a) (u32)(a<<24|b<<16|g<<8|r)

enum
{
	WB_MAX_RLE_PALETTE = 256,

	WB_PIXEL_SHADOW_START = 250,
	WB_PIXEL_SHADOW_END = 254,
	WB_PIXEL_TRANSPARENT = 255,

	WB_PIXEL_SIDE_START = 224,
	WB_PIXEL_SIDE_END = 238,

	VK_FORMAT_R8G8B8_SRGB = 29,
	VK_FORMAT_R8G8B8A8_SRGB = 43,
	VK_FORMAT_BC7_SRGB_BLOCK = 146
};

typedef enum
{
	WB_PALETTE_TYPE_NORMAL = 0,
	WB_PALETTE_TYPE_DAYTIME = 0,
	WB_PALETTE_TYPE_DARKER = 1,
	WB_PALETTE_TYPE_LIGHTER = 2,
	WB_PALETTE_TYPE_NIGHTTIME = 3,
	WB_PALETTE_TYPE_SUNSET1 = 4,
	WB_PALETTE_TYPE_SUNSET2 = 5,
	WB_PALETTE_TYPE_SUNSET3 = 6,
	WB_PALETTE_TYPE_SUNSET4 = 7,
	WB_PALETTE_TYPE_COUNT = 8
} wb_palette_type;

typedef struct
{
	s32 size;
	s16 width;
	s16 height;
	s16 palettes[WB_PALETTE_TYPE_COUNT][WB_MAX_RLE_PALETTE];
	s32 pointer_block_size;
	s32 total_pointer_block_size;
	s16 pointer_block_count;
} wb_rle_header;

static const u8 shadow_a[] = {
		255 - 152, 255 - 216, 255 - 228, 255 - 240, 255 - 248
};

static inline u8 convert_565_to_R(s16 pixel)
{
	return (((u8)((pixel >> 11) & 0x001F)) << 3);
}

static inline u8 convert_565_to_G(s16 pixel)
{
	return (((u8)((pixel >> 5) & 0x003F)) << 2);
}

static inline u8 convert_565_to_B(s16 pixel)
{
	return (((u8)((pixel >> 0) & 0x001F)) << 3);
}

void wb_image_load_bmp(const char* name, const u8* data, u32 size, wb_image* image)
{
	assert(name);
	assert(data);
	assert(image);

	int width, height, component_count;
	u8* rgb_bytes = stbi_load_from_memory(data, size, &width, &height, &component_count, 0);
	assert(rgb_bytes);

	image->width = width;
	image->height = height;
	image->vk_format = VK_FORMAT_BC7_SRGB_BLOCK;

	const u32 rgb_size = width * height * sizeof(u8) * 3;
	image->source_size = rgb_size;
	image->source_data = malloc(rgb_size);
	assert(image->source_data);
	memcpy(image->source_data, rgb_bytes, rgb_size);

	image->name_size = strlen(name);
	image->name = malloc(image->name_size);
	assert(image->name);
	strcpy(image->name, name);

	stbi_image_free(rgb_bytes);
}

/* SSG RLE/RLS encoding:
 *	- Two 255 values mean a blank line
 *  - 250-254 are shadows
 *  - 224-238 are the side colors (15 max)
 */
void wb_image_load_rle(const char* name, const u8* data, wb_image* image)
{
	assert(name);
	assert(data);
	assert(image);

	static const int rle_id_size = 2;

	const bool shadowed = (char)(data[1]) == 'S';
	const wb_rle_header* rle_header = (const wb_rle_header*)(data + rle_id_size);

	s32 rle_data_size = rle_header->size - rle_header->total_pointer_block_size;

	const u8* rle_data = data + rle_id_size + sizeof(wb_rle_header) + rle_header->total_pointer_block_size;

	const u32 rgba_size = rle_header->width * rle_header->height * sizeof(u8) * 4;
	u8* rgba_bytes = malloc(rgba_size);
	assert(rgba_bytes);

	int byte_count = 0;
	for (int rle_data_index = 0; rle_data_index < rle_data_size; rle_data_index++)
	{
		const s16 rle_value = rle_data[rle_data_index];

		if (rle_value == 255)
		{
			// Next value is the number of transparent pixels
			s16 blank_count = rle_data[rle_data_index + 1];

			// Two consecutive 255 means a blank line
			if (rle_data_index < rle_data_size - 1 && blank_count == 255)
			{
				blank_count = rle_header->width;
			}

			memset(rgba_bytes + byte_count, 0, 4 * blank_count);
			byte_count += 4 * blank_count;

			// Advance past the blank count value
			rle_data_index++;
		}
		else if (shadowed && rle_value == WB_PIXEL_SHADOW_START)
		{
			// Next value is the number of shadow pixels
			const s16 shadow_count = rle_data[rle_data_index + 1];
			const u8 alpha = shadow_a[rle_value - WB_PIXEL_SHADOW_START];

			for (int i = 0; i < shadow_count; i++)
			{
				rgba_bytes[byte_count++] = 0;
				rgba_bytes[byte_count++] = 0;
				rgba_bytes[byte_count++] = 0;
				rgba_bytes[byte_count++] = alpha;
			}

			// Advance past the shadow count value
			rle_data_index++;
		}
		else if (shadowed && rle_value > WB_PIXEL_SHADOW_START)
		{
			const u8 alpha = shadow_a[rle_value - WB_PIXEL_SHADOW_START];
			const u32 rgba = pack_rgba(0, 0, 0, alpha);

			memcpy(rgba_bytes + byte_count, &rgba, 4);
			byte_count += 4;
		}
		else
		{
			const s16 color = rle_header->palettes[0][rle_value];
			const u32 rgba = pack_rgba(
					convert_565_to_R(color),
					convert_565_to_G(color),
					convert_565_to_B(color),
					255);

			memcpy(rgba_bytes + byte_count, &rgba, 4);
			byte_count += 4;
		}
	}

	image->width = rle_header->width;
	image->height = rle_header->height;
	image->vk_format = VK_FORMAT_BC7_SRGB_BLOCK - 1;

	image->source_size = rgba_size;
	image->source_data = malloc(rgba_size);
	assert(image->source_data);
	memcpy(image->source_data, rgba_bytes, rgba_size);

	image->name_size = strlen(name);
	image->name = malloc(image->name_size);
	assert(image->name);
	strcpy(image->name, name);

	free(rgba_bytes);
}

void wb_image_free(wb_image* image)
{
	if (image)
	{
		if (image->source_data)
		{
			free(image->source_data);
			image->source_data = NULL;
		}
		if (image->compressed_data)
		{
			free(image->compressed_data);
			image->compressed_data = NULL;
		}
		if (image->name)
		{
			free(image->name);
			image->name = NULL;
		}
	}
}