#include "ktx.h"
#include "image.h"
#include "../../engine/io.h"

#define KHRONOS_STATIC
#include "ktx.h"

#include <string.h>
#include <assert.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

void wb_ktx_create(const wb_image* image)
{
	ktxTextureCreateInfo ktx_create_info = {
			.vkFormat = image->vk_format,
			.baseWidth = image->width,
			.baseHeight = image->height,
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

	result = ktxTexture_SetImageFromMemory(ktxTexture(texture), 0, 0, 0, image->compressed_data, image->compressed_size);
	assert(result == KTX_SUCCESS);

	char ktx_path[MAX_PATH];
	strcpy(ktx_path, image->name);
    path_strip_path(ktx_path);
    io_path_rename_extension(ktx_path, ".ktx2");

	ktxTexture_WriteToNamedFile(ktxTexture(texture), ktx_path);
	ktxTexture_Destroy(ktxTexture(texture));
}
