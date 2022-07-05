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
};

void wb_encode_bc7(wb_image* image)
{
	assert(image);

	const u32 blocks_x = (image->width + 3) >> 2;
	const u32 blocks_y = (image->height + 3) >> 2;
	const u32 block_count = blocks_x * blocks_y;
	const u32 texel_count = block_count * 16;

	bc7e_compress_block_init();

	struct bc7e_compress_block_params block_params = {0};
	bc7e_compress_block_params_init_slow(&block_params, false);

	u64* blocks = malloc(texel_count);
	assert(blocks);

	clock_t start = clock();
    int block_y;
#pragma omp parallel for
	for (block_y = 0; block_y < blocks_y; block_y++)
	{
		for (int block_x = 0; block_x < blocks_x; block_x += WB_BC7_BLOCKS_PER_LOOP)
		{
			const u32 num_blocks_to_process = min(blocks_x - block_x, WB_BC7_BLOCKS_PER_LOOP);

			u32 pixels[16 * WB_BC7_BLOCKS_PER_LOOP] = {0};

			for (int b = 0; b < num_blocks_to_process; b++)
			{
				for (u32 y = 0; y < 4; y++)
				{
					const int index = ((block_x + b) * 4) + image->width * (block_y * 4 + y);
					assert(index < image->source_size);
					memcpy(pixels + b * WB_BC7_BYTES_PER_BLOCK + y * 4, &image->source_data[index], 4 * sizeof(u32));
				}
			}

			int block_index = block_x + block_y * blocks_x;
			assert(block_index < block_count);
			bc7e_compress_blocks(num_blocks_to_process, blocks, (const u32*)pixels, &block_params);
		}
	}
	clock_t end = clock();
	printf("\nTotal encoding time: %f secs\n", (double)(end - start) / CLOCKS_PER_SEC);

	image->compressed_size = texel_count;
	image->compressed_data = malloc(texel_count);
	assert(image->compressed_data);
	memcpy(image->compressed_data, blocks, texel_count);

	free(image->source_data);
	free(blocks);
}
