#ifndef SERVER
#include "asset.h"

typedef struct
{
    char     id[4];
    uint32_t file_count;
} pak_header_t;

typedef struct
{
    char     name[56];
    uint32_t offset;
    uint32_t size;
} pak_file_t;

struct pak_t
{
    pak_file_t* files;
    uint32_t    file_count;
    uint32_t    size;
};

_Static_assert(sizeof(pak_file_t) == 64, "pak_file_t must be 64 bytes.");

void pak_load_from_file(const char* file_name, pak_t* dst)
{

}

void pak_load_from_memory(uint8_t* data, uint32_t data_size, pak_t* dst)
{

}

void pak_unload(pak_t* pak)
{

}
#endif