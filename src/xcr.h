#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    XCR_RESOURCE_UNKNOWN = 0,
    XCR_RESOURCE_BMP,
    XCR_RESOURCE_RLE,
    XCR_RESOURCE_ANI,
    XCR_RESOURCE_TER,
    XCR_RESOURCE_FEA,
    XCR_RESOURCE_ARM,
    XCR_RESOURCE_BUI,
    XCR_RESOURCE_GOM,
    XCR_RESOURCE_WAV,
} xcr_resource_type;

typedef struct
{
    char name[256];
    const uint8_t* data;   // points into the loaded file buffer
    uint32_t size;
    xcr_resource_type type;
} xcr_resource;

typedef struct
{
    uint8_t* file_data;        // entire file in memory (owned)
    size_t file_size;
    xcr_resource* resources;
    uint32_t resource_count;
} xcr_archive;

// Load an XCR archive from disk. Returns NULL on failure.
xcr_archive* xcr_load(const char* path);

// Free all memory associated with an archive.
void xcr_free(xcr_archive* archive);

// Find a resource by name (case-insensitive filename match). Returns NULL if not found.
const xcr_resource* xcr_find(const xcr_archive* archive, const char* name);
