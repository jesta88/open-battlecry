#include "path.h"
#include <string.h>

const char* wb_path_get_extension(const char* path)
{
    u32 length = (u32)strlen(path);
    u32 offset = length - 1;
    while(offset < length) {
        const char c = path[offset];
        if (c == '.')
        {
            return path + offset + 1;
        }
        else if (c == '/' || c == '\\')
        {
            return NULL;
        }
        offset--;
    }
    return NULL;
}