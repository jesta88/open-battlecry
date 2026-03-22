#include "../file_system.h"
#include "../defines.h"
#include "../debug.h"
#include "windows_file.h"
#include "windows_misc.h"

static char base_path[WK_MAX_PATH];

void wk_init_file_system(void)
{
    int path_length = GetModuleFileNameA(NULL, base_path, sizeof(base_path));
    int last_error = GetLastError();
    if (path_length == 0 && last_error != ERROR_SUCCESS) {
        wk_output_debug("Failed to get base path with error: %d", last_error);
    }

    for (char* p = base_path + path_length; p > base_path; p--)
    {
        if (*p == '\\' || *p == '/') {
            *p = 0;
            SetCurrentDirectoryA(base_path);
            break;
        }
    }
}

void wk_quit_file_system(void)
{

}

const char* wk_get_base_path(void)
{
    return base_path;
}
