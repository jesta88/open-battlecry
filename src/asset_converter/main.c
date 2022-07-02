#include "xcr.h"
#include "../common/log.h"
#include "../common/path.h"
#include "../common/string.inl"
#include "../common/filesystem.h"

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

	wb_log_info("XCR loaded: %s", file_path);
	for (u32 i = 0; i < xcr->resource_count; i++)
	{
		const wb_xcr_resource* resource = &xcr->resources[i];
		switch (resource->type)
		{
		case WB_XCR_RESOURCE_BMP:
			int width, height, component_count;
			u8* rgb_bytes = stbi_load_from_memory(resource->data, resource->size, &width, &height, &component_count, 0);

			ktxTexture2* texture;

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
			KTX_error_code result = ktxTexture2_Create(&ktx_create_info, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
			assert(result == KTX_SUCCESS);

			result = ktxTexture_SetImageFromMemory(ktxTexture(texture), 0, 0, 0, rgb_bytes, width * height * component_count);
			assert(result == KTX_SUCCESS);

			char ktx_path[MAX_PATH];
			wb_string_copy(ktx_path, resource->name, MAX_PATH);
			wb_path_strip_path(ktx_path);
			wb_path_rename_extension(ktx_path, ".ktx2");

			ktxTexture_WriteToNamedFile(ktxTexture(texture), ktx_path);
			ktxTexture_Destroy(ktxTexture(texture));

			stbi_image_free(rgb_bytes);
			break;
		}
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
		wb_log_info("User canceled folder selection.");
		return 0;
	}

	wb_xcr_init();
	wb_filesystem_directory_iterate(wbc3_path, file_proc);
	wb_xcr_quit();

	return 0;
}