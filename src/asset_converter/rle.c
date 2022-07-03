#include "rle.h"
#include <malloc.h>
#include <assert.h>
#include <string.h>

#include "../third_party/nvtt/include/nvtt_wrapper.h"

#define KHRONOS_STATIC
#include "../third_party/ktx/include/ktx.h"
#include "../common/string.inl"
#include "../common/path.h"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define BC7_BYTES_PER_TILE 16

#define VK_FORMAT_R8G8B8_SRGB 29
#define VK_FORMAT_R8G8B8A8_SRGB 43
#define VK_FORMAT_BC7_SRGB_BLOCK 146

#define WB_MAX_RLE_PALETTE 256

#define pack_rgba(r, g, b, a) (u32)(a<<24|b<<16|g<<8|r)

enum
{
	PIXEL_SHADOW_START = 250,
	PIXEL_SHADOW_END = 254,
	PIXEL_SHADOW_COUNT = PIXEL_SHADOW_END - PIXEL_SHADOW_START + 1,
	PIXEL_TRANSPARENT = 255,

	PIXEL_SIDE_START = 224,
	PIXEL_SIZE_END = 238
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

static const u8 shadow_r = 0;
static const u8 shadow_g = 0;
static const u8 shadow_b = 0;
static const u8 shadow_a[PIXEL_SHADOW_COUNT] = {
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

/* SSG RLE/RLS encoding:
 *	- Two 255 values mean a blank line
 *  - 250-254 are shadows
 *  - 224-238 are the side colors (15 max)
 */
void wb_rle_to_bc7(const char* name, const u8* rle_bytes)
{
	static const int rle_id_size = 2;

	const bool shadowed = (char)(rle_bytes[1]) == 'S';
	wb_rle_header* rle_header = (wb_rle_header*)(rle_bytes + rle_id_size);

	s32 rle_data_size = rle_header->size - rle_header->total_pointer_block_size;

	const u8* rle_data = rle_bytes + rle_id_size + sizeof(wb_rle_header) + rle_header->total_pointer_block_size;

	u8* rgba_bytes = malloc(rle_header->width * rle_header->height * sizeof(u8) * 4);
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
		else if (shadowed && rle_value == PIXEL_SHADOW_START)
		{
			// Next value is the number of shadow pixels
			const s16 shadow_count = rle_data[rle_data_index + 1];
			const u8 alpha = shadow_a[rle_value - PIXEL_SHADOW_START];

			for (int i = 0; i < shadow_count; i++)
			{
				rgba_bytes[byte_count++] = shadow_r;
				rgba_bytes[byte_count++] = shadow_g;
				rgba_bytes[byte_count++] = shadow_b;
				rgba_bytes[byte_count++] = alpha;
			}

			// Advance past the shadow count value
			rle_data_index++;
		}
		else if (shadowed && rle_value > PIXEL_SHADOW_START)
		{
			const u8 alpha = shadow_a[rle_value - PIXEL_SHADOW_START];
			const u32 rgba = pack_rgba(shadow_r, shadow_g, shadow_b, alpha);

			memcpy(rgba_bytes + byte_count, &rgba, 4);
			byte_count += 4;
		}
		else
		{
			const s16 color = rle_header->palettes[WB_PALETTE_TYPE_NORMAL][rle_value];
			const u32 rgba = pack_rgba(
					convert_565_to_R(color),
					convert_565_to_G(color),
					convert_565_to_B(color),
					255);

			memcpy(rgba_bytes + byte_count, &rgba, 4);
			byte_count += 4;
		}
	}

	// Encode to BC7
	NvttRefImage image = {
			.data = rgba_bytes,
			.width = rle_header->width,
			.height = rle_header->height,
			.depth = 1,
			.num_channels = 4,
			.channel_swizzle = {
					NVTT_ChannelOrder_Red, NVTT_ChannelOrder_Green, NVTT_ChannelOrder_Blue, NVTT_ChannelOrder_Alpha
			},
			.channel_interleave = NVTT_True
	};
	u32 num_tiles;
	NvttCPUInputBuffer* cpu_buffer = nvttCreateCPUInputBuffer(&image, NVTT_ValueType_UINT8, 1, 4, 4, 1.0f, 1.0f, 1.0f, 1.0f, NULL, &num_tiles);
	assert(cpu_buffer);
	free(rgba_bytes);

	u8* bc7_bytes = malloc(num_tiles * BC7_BYTES_PER_TILE);
	assert(bc7_bytes);

	nvttEncodeBC7CPU(cpu_buffer, NVTT_False, NVTT_True, bc7_bytes, nvttIsCudaSupported(), NVTT_False, NULL);

	nvttDestroyCPUInputBuffer(cpu_buffer);

	ktxTextureCreateInfo ktx_create_info = {
			.vkFormat = VK_FORMAT_BC7_SRGB_BLOCK,
			.baseWidth = rle_header->width,
			.baseHeight = rle_header->height,
			.baseDepth = 1,
			.numDimensions = 2,
			.numLevels = 1,
			.numLayers = 1,
			.numFaces = 1,
			.isArray = KTX_FALSE,
			.generateMipmaps = KTX_FALSE,
	};
	ktxTexture2* texture;
	KTX_error_code result = ktxTexture2_Create(&ktx_create_info, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
	assert(result == KTX_SUCCESS);

	result = ktxTexture_SetImageFromMemory(ktxTexture(texture), 0, 0, 0, bc7_bytes, num_tiles * BC7_BYTES_PER_TILE);
	assert(result == KTX_SUCCESS);

	char ktx_path[MAX_PATH];
	wb_string_copy(ktx_path, name, MAX_PATH);
	wb_path_strip_path(ktx_path);
	wb_path_rename_extension(ktx_path, ".ktx2");

	ktxTexture_WriteToNamedFile(ktxTexture(texture), ktx_path);
	ktxTexture_Destroy(ktxTexture(texture));

	free(bc7_bytes);
}
