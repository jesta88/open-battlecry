#pragma once

#include "../../engine/std.h"

typedef enum
{
    WB_XCR_RESOURCE_NULL,
    WB_XCR_RESOURCE_BMP,
    WB_XCR_RESOURCE_RLE,
    WB_XCR_RESOURCE_ANI,
    WB_XCR_RESOURCE_TER,
    WB_XCR_RESOURCE_FEA,
    WB_XCR_RESOURCE_ARM,
    WB_XCR_RESOURCE_BUI,
    WB_XCR_RESOURCE_GOM,
    WB_XCR_RESOURCE_WAV,
    WB_XCR_RESOURCE_COUNT
} wb_xcr_resource_type;

typedef struct
{
	const char* name;
	const u8* data;
	const u32 size;
	const u8 type;
} wb_xcr_resource;

typedef struct 
{
    u64 resource_count;
    wb_xcr_resource* resources;
} wb_xcr;

void wb_xcr_init(void);
void wb_xcr_quit(void);

wb_xcr* wb_xcr_load(const char* path);
void wb_xcr_unload(wb_xcr* xcr);

const char* wb_xcr_resource_type_string(wb_xcr_resource* resource);