#include "io.h"
#include "engine_private.h"
#include "log.h"
#include <string.h>
#ifdef _WIN32
#include <shlwapi.h>
#include <shlobj.h>
#endif

#ifdef _WIN32
static BOOL CALLBACK browse_callback_proc_enum(HWND hWndChild, LPARAM lParam);
static int CALLBACK browse_callback_proc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData);
#endif

void io_init(void)
{
    int32_t length;
    bool result;

    // Base path
#ifdef _WIN32
    length = GetModuleFileNameA(NULL, engine.base_path, MAX_PATH);
#endif
    if (length == 0)
    {
        log_error("Could not get base path.");
        return;
    }
    io_path_remove_file_name(engine.base_path);

    // User path
#ifdef _WIN32
    PWSTR wide_user_path = NULL;
    result = SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_RoamingAppData, KF_FLAG_CREATE, NULL, &wide_user_path));
    if (result)
    {
        WideCharToMultiByte(CP_UTF8, 0, wide_user_path, -1, engine.user_path, MAX_PATH, NULL, NULL);
    }
#endif
    if (!result)
    {
        log_error("Could not get user path.");
    }
#ifdef _WIN32
    CoTaskMemFree(wide_user_path);
#endif
}

void io_path_remove_path(char* path)
{
#ifdef _WIN32
    PathStripPathA(path);
#endif
}

void io_path_remove_file_name(char* path)
{
    uint64_t length = strlen(path);
    int i;
    for (i = length - 1; i > 0; i--)
    {
        if (path[i] == '\\')
        {
            break;
        }
    }
    path[i + 1] = '\0';
}

void io_path_remove_extension(char* path)
{
#ifdef _WIN32
    PathRemoveExtensionA(path);
#endif
}

void io_path_rename_extension(char* path, const char* extension)
{
    bool result;
#ifdef _WIN32
    result = PathRenameExtensionA(path, extension);
#endif
    if (!result)
    {
        log_error("Could not rename path extension. Path: %s, Extension: %s", path, extension);
    }
}

const char* io_path_find_extension(const char* path)
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

uint32_t io_file_read(const char* path, uint8_t* buffer)
{
    uint32_t bytes_read;
#ifdef _WIN32
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (file == NULL)
    {
        log_error("Could not open file at path %s", path);
        return 0;
    }

    ULARGE_INTEGER file_size = {0};
    file_size.LowPart = GetFileSize(file, &file_size.HighPart);

    DWORD bytes_read_dword;
    if (ReadFile(file, buffer, file_size.LowPart, &bytes_read_dword, NULL) == FALSE)
    {
        log_error("Could not read file at path %s: %ld", path, GetLastError());
        CloseHandle(file);
        return 0;
    }
    bytes_read = (uint32_t)bytes_read_dword;
    CloseHandle(file);
#endif
    return bytes_read;
}

const char* io_file_dialog(const char* title, const char* default_path)
{
    static char buffer[MAX_PATH];
    char* result;
#ifdef _WIN32
    HRESULT hresult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    BROWSEINFOA browse_info = {0};
    browse_info.hwndOwner = GetForegroundWindow();
    browse_info.pidlRoot = NULL;
    browse_info.pszDisplayName = buffer;
    browse_info.lpszTitle = title;
    if (hresult == S_OK || hresult == S_FALSE)
    {
        browse_info.ulFlags = BIF_USENEWUI;
    }
    browse_info.lpfn = browse_callback_proc;
    browse_info.lParam = (LPARAM)default_path;
    browse_info.iImage = -1;

    LPITEMIDLIST id_list = SHBrowseForFolderA(&browse_info);
    if (!id_list)
    {
        result = NULL;
    }
    else
    {
        SHGetPathFromIDListA(id_list, buffer);
        result = buffer;
    }

    if (hresult == S_OK || hresult == S_FALSE)
    {
        CoUninitialize();
    }
#endif
    return result;
}

#ifdef _WIN32
static BOOL CALLBACK browse_callback_proc_enum(HWND hWndChild, LPARAM lParam)
{
    (void)lParam;
    char buffer[255];
    GetClassNameA(hWndChild, buffer, sizeof(buffer));
    if (strcmp(buffer, "SysTreeView32") == 0)
    {
        HTREEITEM hNode = TreeView_GetSelection(hWndChild);
        TreeView_EnsureVisible(hWndChild, hNode);
        return FALSE;
    }
    return TRUE;
}

static int CALLBACK browse_callback_proc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
    (void)lp;
    switch (uMsg)
    {
        case BFFM_INITIALIZED:
            SendMessageA(hwnd, BFFM_SETSELECTIONA, TRUE, (LPARAM)pData);
            break;
        case BFFM_SELCHANGED:
            EnumChildWindows(hwnd, browse_callback_proc_enum, 0);
    }
    return 0;
}

//void io_directory_iterate(const char* path, void (*file_proc)(const char*))
//{
//    char search_path[MAX_PATH];
//    strncpy(search_path, path, MAX_PATH);
//    strcat(search_path, "\\*");
//
//    WIN32_FIND_DATAA find_data;
//    HANDLE find_handle = FindFirstFileA(search_path, &find_data);
//    if (find_handle == INVALID_HANDLE_VALUE)
//    {
//        return;
//    }
//
//    int subdir_count = 0;
//    // TODO: Remove ugly magic number
//    char subdirs[28][MAX_PATH];
//
//    do
//    {
//        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//        {
//            char* dir_name = find_data.cFileName;
//
//            if (dir_name[0] != '.')
//            {
//                strncpy(subdirs[subdir_count], path, MAX_PATH);
//                strcat(subdirs[subdir_count], "\\");
//                strcat(subdirs[subdir_count], dir_name);
//                subdir_count++;
//            }
//        }
//        else
//        {
//            char file_path[MAX_PATH];
//            strncpy(file_path, path, MAX_PATH);
//            strcat(file_path, "\\");
//            strcat(file_path, find_data.cFileName);
//
//            file_proc(file_path);
//        }
//    } while (FindNextFileA(find_handle, &find_data) != 0);
//
//    for (int i = 0; i < subdir_count; i++)
//    {
//        io_directory_iterate(subdirs[i], file_proc);
//    }
//
//    FindClose(find_handle);
//}
#endif