#include "../os.h"
#include "../debug.h"
#include "../engine.h"
#include "windows_io.h"
#include "windows_misc.h"
#include "windows_process.h"
#include <assert.h>

void wk_init_os(void)
{
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
    SetSearchPathMode(BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT);
    SetDllDirectoryA("");

    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    const HINSTANCE hinstance = wk_get_win32_hinstance();
    const DWORD priority = GetPriorityClass(hinstance);
    if (priority == NORMAL_PRIORITY_CLASS || priority == ABOVE_NORMAL_PRIORITY_CLASS)
    {
        SetPriorityClass(hinstance, HIGH_PRIORITY_CLASS);
    }
}

void* wk_load_library(const char* name)
{
    if (!name) {
        wk_log_error("Invalid parameter: name");
        return NULL;
    }
    const HMODULE handle = LoadLibraryA(name);
    if (!handle) {
        wk_log_error("Failed loading %s", name);
    }

    return handle;
}

void wk_unload_library(void* library)
{
    assert(library);
    FreeLibrary(library);
}

void* wk_load_function(void* library, const char* name)
{
    void* func = (void*)GetProcAddress(library, name);
    if (!func) {
        wk_log_error("Failed loading %s", name);
    }
    return func;
}