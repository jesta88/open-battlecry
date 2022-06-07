#ifndef SERVER
#include "asset.h"

typedef struct
{
    char     id[4];
    u32 file_count;
} pak_header_t;

typedef struct
{
    char     name[56];
    u32 offset;
    u32 size;
} pak_file_t;

struct pak_t
{
    pak_file_t* files;
    u32    file_count;
    u32    size;
};

_Static_assert(sizeof(pak_file_t) == 64, "pak_file_t must be 64 bytes.");

void pak_load_from_file(const char* file_name, pak_t* dst)
{

}

void pak_load_from_memory(u8* data, u32 data_size, pak_t* dst)
{

}

void pak_unload(pak_t* pak)
{

}
#endif