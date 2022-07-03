#include "bmp.h"

#include "../common/path.h"
#include "../common/string.inl"

#define STBI_ONLY_BMP
#define STBI_NO_STDIO
#include "../third_party/stb_image.h"

#define KHRONOS_STATIC
#include "../third_party/ktx/include/ktx.h"

#include <assert.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define VK_FORMAT_R8G8B8_SRGB 29
#define VK_FORMAT_BC7_SRGB_BLOCK = 146

void wb_bmp_to_rgb(const char* name, u32 bmp_size, const u8* bmp_bytes)
{
	int width, height, component_count;
	u8* rgb_bytes = stbi_load_from_memory(bmp_bytes, bmp_size, &width, &height, &component_count, 0);

	ktxTextureCreateInfo ktx_create_info = {
			.vkFormat = VK_FORMAT_R8G8B8_SRGB,
			.baseWidth = width,
			.baseHeight = height,
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

	result = ktxTexture_SetImageFromMemory(ktxTexture(texture), 0, 0, 0, rgb_bytes, width * height * component_count);
	assert(result == KTX_SUCCESS);

	char ktx_path[MAX_PATH];
	wb_string_copy(ktx_path, name, MAX_PATH);
	wb_path_strip_path(ktx_path);
	wb_path_rename_extension(ktx_path, ".ktx2");

	ktxTexture_WriteToNamedFile(ktxTexture(texture), ktx_path);
	ktxTexture_Destroy(ktxTexture(texture));

	stbi_image_free(rgb_bytes);
}
