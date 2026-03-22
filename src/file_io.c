#include "file_io.h"

#include <stdio.h>
#include <stdlib.h>

uint8_t* read_file_binary(const char* path, size_t* out_size)
{
    FILE* f = fopen(path, "rb");
    if (!f)
    {
        fprintf(stderr, "[file_io] Failed to open: %s\n", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (len <= 0)
    {
        fclose(f);
        return NULL;
    }

    uint8_t* buf = malloc((size_t)len);
    if (!buf)
    {
        fclose(f);
        return NULL;
    }

    size_t read = fread(buf, 1, (size_t)len, f);
    fclose(f);

    if (read != (size_t)len)
    {
        free(buf);
        return NULL;
    }

    *out_size = (size_t)len;
    return buf;
}
