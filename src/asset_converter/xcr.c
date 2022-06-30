#include "xcr.h"
#include "../common/log.h"
#include "../common/memory.h"
#include "../common/filesystem.h"
#include "../common/string.inl"
#include <string.h>
#include <malloc.h>

static const char* xcr_id = "xcr File 1.00";

typedef struct
{
	char identifier[20];
	s32 resource_count;
	u32 file_size;
} wb_xcr_header;

typedef struct
{
	char file_name[256];
	char full_path[256];
	u32 offset;
	u32 file_size;
	s32 file_type;
	u32 crc_block;
	bool crc_check;
	bool encrypted;
} wb_xcr_resource_header;

enum
{
	WB_XCR_MAX_RESOURCES = 256
};

typedef enum
{
	WB_XCR_RESOURCE_BMP,
	WB_XCR_RESOURCE_RLE,
	WB_XCR_RESOURCE_ANI,
	WB_XCR_RESOURCE_TER
} wb_xcr_resource_type;

typedef struct
{
	int offset;
	u8 type;
	char name[59];
} wb_xcr_resource;

struct wb_xcr
{
	wb_file_mapping mapping;
	wb_xcr_resource resources[WB_XCR_MAX_RESOURCES];
};

wb_xcr* wb_xcr_load(const char* path)
{
	wb_xcr* xcr = malloc(sizeof(wb_xcr));
	if (!xcr)
		return NULL;

	wb_memory_map(path, &xcr->mapping);

	uintptr_t current_ptr = (uintptr_t)xcr->mapping.data;

	const wb_xcr_header* xcr_header = (wb_xcr_header*)current_ptr;
	current_ptr += sizeof(wb_xcr_header);

	// Read resource headers
	for (int i = 0; i < xcr_header->resource_count; i++)
	{
		const wb_xcr_resource_header* resource_header = (wb_xcr_resource_header*)current_ptr;
		current_ptr += sizeof(wb_xcr_resource_header);

		char extension[3];
		if (wb_str_get_extension(resource_header->file_name, extension) != 0)
			continue;

		if (strncmp(extension, "rle", 3) == 0)
		{
//			image_offsets[image_count] = resource_header.offset;
//			strncpy(image_names[image_count], resource_header.file_name, strlen(resource_header.file_name));
//			++image_count;
		}
		else if (strncmp(extension, "bmp", 3) == 0)
		{

		}
		else if (strncmp(extension, "ani", 3) == 0)
		{

		}
		else if (strncmp(extension, "ter", 3) == 0)
		{

		}
	}

	char* xcr_name = strrchr(path, '\\');
	xcr_name++;
	char* dot = strrchr(path, '.');
	*dot = '\0';

	char snake_xcr_name[32];
	wb_str_to_snake_case(snake_xcr_name, 32, xcr_name);

//	char out_dir[256];
//	strcpy(out_dir, output_directory);
//	strcat(out_dir, "\\");
//	strcat(out_dir, wcs_xcr_name);
//	_mkdir(out_dir);

	// Process resources
//    for (u32 i = 0; i < image_count; i++) {
//        process_rle(i, snake_xcr_name);
//    }

	return xcr;
}

void wb_xcr_unload(wb_xcr* xcr)
{
	if (xcr == NULL)
		return;
	wb_memory_unmap(&xcr->mapping);
	free(xcr);
}