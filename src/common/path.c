#include "path.h"
#include <string.h>
#ifdef _WIN64
#include <shlwapi.h>
#endif
#include <assert.h>

const char* wb_path_get_extension(const char* path)
{
    u32 length = (u32)strlen(path);
    u32 offset = length - 1;
    while(offset < length)
    {
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

void wb_path_rename_extension(char* path, const char* extension)
{
    BOOL result = PathRenameExtensionA(path, extension);
    assert(result != 0);
}

void wb_path_strip_path(char* path)
{
    PathStripPathA(path);
}

const char* wb_path_get_file_name_with_extension(const char* path)
{
    return NULL;
}
