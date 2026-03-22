#include "xcr.h"
#include "file_io.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// On-disk structures (must match Delphi packed records)
// ---------------------------------------------------------------------------

// On-disk layout matches MSVC default packing (4-byte aligned).
// xBOOL in the original Delphi/C++ code is typedef'd as char (1 byte).
// The struct ends with two 1-byte bools, then MSVC pads to 4-byte alignment = 532 bytes total.

typedef struct
{
    char identifier[20];
    int32_t resource_count;
    uint32_t file_size;
} xcr_file_header;

typedef struct
{
    char file_name[256];
    char full_path[256];
    uint32_t offset;
    uint32_t file_size;
    int32_t file_type;
    uint32_t crc_block;
    char crc_check;
    char encrypted;
    char _pad[2]; // MSVC 4-byte struct alignment padding
} xcr_resource_header;

// ---------------------------------------------------------------------------
// XCR decryption (byte substitution cipher with seeded LCG)
// ---------------------------------------------------------------------------

static uint8_t g_decrypt_table[256];
static bool g_decrypt_initialized = false;

static void init_decrypt_table(void)
{
    if (g_decrypt_initialized) return;

    uint8_t encrypt_table[256];
    uint32_t last = 728; // seed
    const uint32_t modulus = 65536;
    const uint32_t increment = 13849;
    const uint32_t multiplier = 25173;
    const uint32_t rand_max = 65535;

    for (int i = 0; i < 256; ++i)
    {
        bool finished = false;
        while (!finished)
        {
            uint32_t res = (multiplier * last + increment) % modulus;
            last = res;
            uint32_t r = (res * 256) / rand_max;
            if (r > 255) r = 255;

            bool found = false;
            for (int j = 0; j < i; ++j)
                if (encrypt_table[j] == (uint8_t)r) { found = true; break; }

            if (!found)
            {
                finished = true;
                encrypt_table[i] = (uint8_t)r;
                g_decrypt_table[(uint8_t)r] = (uint8_t)i;
            }
        }
    }
    g_decrypt_initialized = true;
}

static void decrypt_data(uint8_t* data, uint32_t size)
{
    init_decrypt_table();
    for (uint32_t i = 0; i < size; ++i)
        data[i] = g_decrypt_table[data[i]];
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static xcr_resource_type detect_type(const char* filename)
{
    const char* dot = NULL;
    for (const char* p = filename; *p; ++p)
        if (*p == '.') dot = p;
    if (!dot || !dot[1] || !dot[2])
        return XCR_RESOURCE_UNKNOWN;

    char ext[4];
    ext[0] = (char)tolower((unsigned char)dot[1]);
    ext[1] = (char)tolower((unsigned char)dot[2]);
    ext[2] = (char)tolower((unsigned char)dot[3]);
    ext[3] = '\0';

    if (strcmp(ext, "bmp") == 0) return XCR_RESOURCE_BMP;
    if (strcmp(ext, "rle") == 0) return XCR_RESOURCE_RLE;
    if (strcmp(ext, "ani") == 0) return XCR_RESOURCE_ANI;
    if (strcmp(ext, "ter") == 0) return XCR_RESOURCE_TER;
    if (strcmp(ext, "fea") == 0) return XCR_RESOURCE_FEA;
    if (strcmp(ext, "arm") == 0) return XCR_RESOURCE_ARM;
    if (strcmp(ext, "bui") == 0) return XCR_RESOURCE_BUI;
    if (strcmp(ext, "gom") == 0) return XCR_RESOURCE_GOM;
    if (strcmp(ext, "wav") == 0) return XCR_RESOURCE_WAV;
    return XCR_RESOURCE_UNKNOWN;
}

static int stricmp_portable(const char* a, const char* b)
{
    while (*a && *b)
    {
        int ca = tolower((unsigned char)*a++);
        int cb = tolower((unsigned char)*b++);
        if (ca != cb) return ca - cb;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

xcr_archive* xcr_load(const char* path)
{
    size_t file_size = 0;
    uint8_t* file_data = read_file_binary(path, &file_size);
    if (!file_data)
    {
        fprintf(stderr, "[xcr] Failed to read: %s\n", path);
        return NULL;
    }

    if (file_size < sizeof(xcr_file_header))
    {
        fprintf(stderr, "[xcr] File too small: %s\n", path);
        free(file_data);
        return NULL;
    }

    const xcr_file_header* header = (const xcr_file_header*)file_data;

    if (strncmp(header->identifier, "xcr File", 8) != 0)
    {
        fprintf(stderr, "[xcr] Invalid XCR identifier: %s\n", path);
        free(file_data);
        return NULL;
    }

    int32_t total_resources = header->resource_count;
    size_t headers_end = sizeof(xcr_file_header) + (size_t)total_resources * sizeof(xcr_resource_header);
    if (headers_end > file_size)
    {
        fprintf(stderr, "[xcr] Resource headers exceed file size: %s\n", path);
        free(file_data);
        return NULL;
    }

    // First pass: count valid resources
    uint32_t valid_count = 0;
    const uint8_t* ptr = file_data + sizeof(xcr_file_header);
    for (int32_t i = 0; i < total_resources; ++i)
    {
        const xcr_resource_header* rh = (const xcr_resource_header*)ptr;
        ptr += sizeof(xcr_resource_header);
        xcr_resource_type t = detect_type(rh->file_name);
        if (t != XCR_RESOURCE_UNKNOWN)
            valid_count++;
    }

    xcr_archive* archive = calloc(1, sizeof(xcr_archive));
    archive->file_data = file_data;
    archive->file_size = file_size;
    archive->resources = calloc(valid_count, sizeof(xcr_resource));
    archive->resource_count = 0;

    // Second pass: populate resources
    ptr = file_data + sizeof(xcr_file_header);
    for (int32_t i = 0; i < total_resources; ++i)
    {
        const xcr_resource_header* rh = (const xcr_resource_header*)ptr;
        ptr += sizeof(xcr_resource_header);

        xcr_resource_type type = detect_type(rh->file_name);
        if (type == XCR_RESOURCE_UNKNOWN)
            continue;

        xcr_resource* res = &archive->resources[archive->resource_count++];
        memcpy(res->name, rh->file_name, 256);
        res->data = file_data + rh->offset;
        res->size = rh->file_size;
        res->type = type;

        // Decrypt in-place if encrypted
        if (rh->encrypted && rh->offset + rh->file_size <= file_size)
            decrypt_data(file_data + rh->offset, rh->file_size);
    }

    fprintf(stderr, "[xcr] Loaded %s: %u resources (%u total in file)\n",
            path, archive->resource_count, (uint32_t)total_resources);

    return archive;
}

void xcr_free(xcr_archive* archive)
{
    if (!archive) return;
    free(archive->file_data);
    free(archive->resources);
    free(archive);
}

const xcr_resource* xcr_find(const xcr_archive* archive, const char* name)
{
    if (!archive || !name) return NULL;
    for (uint32_t i = 0; i < archive->resource_count; ++i)
    {
        if (stricmp_portable(archive->resources[i].name, name) == 0)
            return &archive->resources[i];
    }
    return NULL;
}
