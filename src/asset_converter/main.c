#include "xcr.h"
#include "image.h"
#include "encode.h"
#include "ktx.h"
#include "../common/path.h"
#include "../common/filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// TODO: Replace folder iteration with static list of xcr
// Fill huge image buffer for all xcr, then encode, then create ktx

static void file_proc(const char* file_path)
{
	const char* extension = wb_path_get_extension(file_path);
	if (extension == NULL || extension[0] != 'x' || extension[1] != 'c' || extension[2] != 'r')
	{
		return;
	}

	wb_xcr* xcr = wb_xcr_load(file_path);
	if (xcr == NULL)
	{
		return;
	}
	printf("\nXCR loaded: %s\n", file_path);
	fflush(stdout);

	// Count the number of images to allocate
	u32 image_count = 0;
	for (u32 i = 0; i < xcr->resource_count; i++)
	{
		if (xcr->resources[i].type == WB_XCR_RESOURCE_BMP ||
			xcr->resources[i].type == WB_XCR_RESOURCE_RLE)
		{
			image_count++;
		}
	}

	// Allocate image buffer
	wb_image* image_buffer = NULL;
	if (image_count > 0)
	{
		image_buffer = malloc(image_count * sizeof(wb_image));
		assert(image_buffer);
	}

	// Process resources
	image_count = 0;
	for (u32 i = 0; i < 2; i++)
	{
		const wb_xcr_resource* resource = &xcr->resources[i];
		switch (resource->type)
		{
		case WB_XCR_RESOURCE_BMP:
		{
			wb_image* image = &image_buffer[image_count++];
			wb_image_load_bmp(resource->name, resource->data, resource->size, image);
			wb_encode_bc7(image);
			wb_ktx_create(image);
			break;
		}
		case WB_XCR_RESOURCE_RLE:
		{
			wb_image* image = &image_buffer[image_count++];
			wb_image_load_rle(resource->name, resource->data, image);
			wb_encode_bc7(image);
			wb_ktx_create(image);
			break;
		}
		}
	}

	if (image_buffer)
	{
		free(image_buffer);
	}

	wb_xcr_unload(xcr);
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	const char* wbc3_path = wb_filesystem_folder_dialog("Warlords Battlecry 3 folder", "C:\\Games\\Warlords Battlecry 3");
	if (wbc3_path == NULL)
	{
		printf("\nUser canceled folder selection.\n");
		return 0;
	}

	wb_xcr_init();
	wb_filesystem_directory_iterate(wbc3_path, file_proc);
	wb_xcr_quit();

	return 0;
}