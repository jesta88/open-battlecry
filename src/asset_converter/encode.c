#include "encode.h"
#include "image.h"

#include "../third_party/bc7enc_rdo/bc7e_ispc.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

enum
{
	WB_BC7_BYTES_PER_BLOCK = 16,
	WB_BC7_BLOCKS_PER_LOOP = 64,

 	VK_FORMAT_R8G8B8_SRGB = 29,
 	VK_FORMAT_R8G8B8A8_SRGB = 43,
	VK_FORMAT_BC7_SRGB_BLOCK = 146
};

typedef struct
{
	u64 values[2];
} wb_block_16;

void wb_encode_bc7(wb_image* image)
{
	assert(image);

	const u32 blocks_x = image->width / 4;
	const u32 blocks_y = image->height / 4;
	const u32 total_blocks = blocks_x * blocks_y;
	const u32 total_texels = total_blocks * WB_BC7_BYTES_PER_BLOCK;

	bc7e_compress_block_init();

	struct bc7e_compress_block_params block_params = {0};
	bc7e_compress_block_params_init_slow(&block_params, false);

	wb_block_16* blocks = malloc(total_blocks);
	assert(blocks);

	clock_t start = clock();
//#pragma omp parallel for
	for (int block_y = 0; block_y < blocks_y; block_y++)
	{
		unsigned int block_x;
		for (block_x = 0; block_x < blocks_x; block_x += WB_BC7_BLOCKS_PER_LOOP)
		{
			const u32 num_blocks_to_process = min(blocks_x - block_x, WB_BC7_BLOCKS_PER_LOOP);

			u32 pixels[16 * WB_BC7_BLOCKS_PER_LOOP];

			for (int b = 0; b < num_blocks_to_process; b++)
			{
				for (u32 y = 0; y < 4; y++)
				{
					const int index = ((block_x + b) * 4) + image->width * (block_y * 4 + y);
					assert(index < image->source_size);
					memcpy(pixels + b * 16 + y * 4, &image->source_data[index], 4 * sizeof(u32));
				}
			}

			int block_index = block_x + block_y * blocks_x;
			assert(block_index < total_blocks);
			wb_block_16* block = &blocks[block_index];
			bc7e_compress_blocks(num_blocks_to_process, (u64*)block, (const u32*)pixels, &block_params);
		}
	}
	clock_t end = clock();
	printf("\nTotal encoding time: %f secs\n", (double)(end - start) / CLOCKS_PER_SEC);

	image->compressed_size = total_texels;
	image->compressed_data = malloc(total_texels);
	assert(image->compressed_data);
	memcpy(image->compressed_data, blocks, total_texels);

	free(blocks);
}
