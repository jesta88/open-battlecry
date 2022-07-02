#include "xcr.h"
#include "../common/log.h"
#include "../common/path.h"
#include "../common/memory.h"
#include "../common/filesystem.h"
#include "../common/string.inl"
#include <string.h>
#include <malloc.h>
#include <assert.h>

enum
{
    XCR_MAX_SIZE = 4000 * 17500 // 70 MB worth of 4 KB pages
};

typedef struct
{
    char identifier[20];
    s32 resource_count;
    u32 file_size;
} wb_xcr_header;

typedef enum
{
    FILE_TYPE_DATA,
    FILE_TYPE_TEXT,
    FILE_TYPE_STRING
} wb_xcr_resource_file_type;

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

static const char* xcr_id = "xcr File 1.00";
static const u64 k_hash_seed = 7127;
static const char* resource_extensions[] = { "", "bmp", "rle", "ani", "ter", "fea", "arm", "bui", "gom", "wav" };

static bool initialized;
static u8* xcr_buffer;

static u64 resource_extension_hashes[WB_XCR_RESOURCE_COUNT];
static wb_xcr_resource_type resource_hash_index_to_type[WB_XCR_RESOURCE_COUNT];

static inline u64 hash_extension(const char extension[3])
{
    return (u64) (extension[0] * extension[1] * extension[2]);
}

void wb_xcr_init(void)
{
    xcr_buffer = wb_memory_virtual_alloc(XCR_MAX_SIZE);

    initialized = true;
}

void wb_xcr_quit(void)
{
    wb_memory_virtual_free(xcr_buffer);
    initialized = false;
}

wb_xcr* wb_xcr_load(const char* path)
{
    if (!initialized)
    {
        wb_log_error("XCR system not initialized. Call wb_xcr_init()");
        return NULL;
    }

    u32 buffer_size = wb_filesystem_read(path, xcr_buffer);
    if (buffer_size == 0)
    {
        return NULL;
    }

    wb_xcr* xcr_data = malloc(sizeof(wb_xcr));
    assert(xcr_data);

    uintptr_t current_ptr = (uintptr_t) xcr_buffer;

    const wb_xcr_header* xcr_header = (wb_xcr_header*) current_ptr;
    
    xcr_data->resources = calloc(xcr_header->resource_count, sizeof(wb_xcr_resource));
    assert(xcr_data->resources);

    current_ptr += sizeof(wb_xcr_header);

    // Read resource headers
    u32 resource_count = 0;
    for (int i = 0; i < xcr_header->resource_count; i++)
    {
        const wb_xcr_resource_header* resource_header = (wb_xcr_resource_header*) current_ptr;
        current_ptr += sizeof(wb_xcr_resource_header);

        const char* extension = wb_path_get_extension(resource_header->file_name);
        if (extension == NULL)
        {
            continue;
        }

        u8 resource_type = U8_MAX;
        if (extension[0] == 'b' && extension[1] == 'm') resource_type = (u8) WB_XCR_RESOURCE_BMP;
        else if (extension[0] == 'r' && extension[1] == 'l') resource_type = (u8) WB_XCR_RESOURCE_RLE;
        else if (extension[0] == 'a' && extension[1] == 'n') resource_type = (u8) WB_XCR_RESOURCE_ANI;
        else if (extension[0] == 't' && extension[1] == 'e') resource_type = (u8) WB_XCR_RESOURCE_TER;
        else if (extension[0] == 'f' && extension[1] == 'e') resource_type = (u8) WB_XCR_RESOURCE_FEA;
        else if (extension[0] == 'a' && extension[1] == 'r') resource_type = (u8) WB_XCR_RESOURCE_ARM;
        else if (extension[0] == 'b' && extension[1] == 'u') resource_type = (u8) WB_XCR_RESOURCE_BUI;
        else if (extension[0] == 'g' && extension[1] == 'o') resource_type = (u8) WB_XCR_RESOURCE_GOM;
        else if (extension[0] == 'w' && extension[1] == 'a') resource_type = (u8) WB_XCR_RESOURCE_WAV;
        else
        {
            continue;
        }

        //wb_string_copy(xcr_data->resources[resource_count].name, resource_header->file_name, strlen(resource_header->file_name));
        const wb_xcr_resource resource = {
                .name = resource_header->file_name,
                .data = xcr_buffer + resource_header->offset,
                .size = xcr_header->file_size,
                .type = resource_type
        };
        memcpy(&xcr_data->resources[resource_count], &resource, sizeof(wb_xcr_resource));
        resource_count++;        
    }
    xcr_data->resource_count = resource_count;

    return xcr_data;
}

void wb_xcr_unload(wb_xcr* xcr)
{
    if (!initialized)
    {
        wb_log_error("XCR system not initialized. Call wb_xcr_init()");
        return;
    }

    free(xcr->resources);
    free(xcr);
    xcr = NULL;
}

const char* wb_xcr_resource_type_string(wb_xcr_resource* resource)
{
    assert(resource);
    return resource_extensions[resource->type];
}