#include "path.h"
#include "string.inl"
#include <string.h>
#include <assert.h>

void path_extension(const char* path, char* extension)
{
    const size_t path_length = strlen(path);
    assert(path_length != 0);

    const char* dot = strrchr(path, '.');
    if (dot == NULL)
    {
        return;
    }

    const char* path_extension = dot + 1;
    const size_t extension_length = strlen(path_extension);
    if (extension_length == 0 || path_extension[0] == '/' || path_extension[0] == '\\')
    {
        return;
    }

    string_copy(extension, path_extension, extension_length);
}
