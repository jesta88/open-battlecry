#include "strings.h"
#include <string.h>

void str_remove_path(char* path)
{
    uint64_t length = strlen(path);
    int i;
    for (i = length - 1; i > 0; i--)
    {
        if (path[i] == '\\' || path[i] == '/')
        {
            break;
        }
    }
    path = path + i + 1;
}

void str_remove_file_name(char* path)
{
    uint64_t length = strlen(path);
    int i;
    for (i = length - 1; i > 0; i--)
    {
        if (path[i] == '\\' || path[i] == '/')
        {
            break;
        }
    }
    path[i + 1] = '\0';
}

void str_remove_extension(char* path)
{
    uint64_t length = strlen(path);
    int i;
    for (i = length - 1; i > 0; i--)
    {
        if (path[i] == '.')
        {
            break;
        }
    }
    path[i] = '\0';
}

void str_rename_extension(char* path, const char* extension)
{
    uint64_t length = strlen(path);
    int i;
    for (i = length - 1; i > 0; i--)
    {
        if (path[i] == '.')
        {
            break;
        }
    }
    path[i + 1] = '\0';
    strcat(path, extension);
}

const char* str_find_extension(const char* path)
{
    uint64_t length = strlen(path);
    uint64_t offset = length - 1;
    while (offset < length)
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