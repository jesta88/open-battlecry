#include "filesystem.h"
#include "string.inl"
#include "log.h"
#if 1
#include <shlobj.h>
#include <commctrl.h>
#else
#include "../common/windows_obj.h"
#include "../common/windows_user.h"
#include "../common/windows_file.h"
#include "../common/windows_window.h"
#endif
#include <assert.h>

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

static BOOL CALLBACK browse_callback_proc_enum(HWND hWndChild, LPARAM lParam)
{
    char buffer[255];
    GetClassNameA(hWndChild, buffer, sizeof(buffer));
    if (strcmp(buffer, "SysTreeView32") == 0) {
        HTREEITEM hNode = TreeView_GetSelection(hWndChild);
        TreeView_EnsureVisible(hWndChild, hNode);
        return FALSE;
    }
    return TRUE;
}

static int CALLBACK browse_callback_proc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
    switch (uMsg) {
        case BFFM_INITIALIZED:
            SendMessageA(hwnd, BFFM_SETSELECTIONA, TRUE, (LPARAM) pData);
            break;
        case BFFM_SELCHANGED:
            EnumChildWindows(hwnd, browse_callback_proc_enum, 0);
    }
    return 0;
}

u32 wb_filesystem_read(const char* path, u8* buffer)
{
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    assert(file != INVALID_HANDLE_VALUE);

    ULARGE_INTEGER file_size = {0};
    file_size.LowPart = GetFileSize(file, &file_size.HighPart);

    DWORD bytes_read;
    //OVERLAPPED overlapped = {0};
    if (ReadFile(file, buffer, file_size.LowPart, &bytes_read, NULL) == FALSE)
    {
        wb_log_error("Could not read file %s: %d", path, GetLastError());
        CloseHandle(file);
        return 0;
    }

    return bytes_read;

    CloseHandle(file);

}

const char* wb_filesystem_folder_dialog(const char* title, const char* default_path)
{
    static char buffer[MAX_PATH];
    char* result;

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
    browse_info.lParam = (LPARAM) default_path;
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
    return result;
}

void wb_filesystem_directory_iterate(const char* path, void(*file_proc)(const char*))
{
    char search_path[MAX_PATH];
    wb_string_copy(search_path, path, MAX_PATH);
    strcat(search_path, "\\*");

    WIN32_FIND_DATAA find_data;
    HANDLE find_handle = FindFirstFileA(search_path, &find_data);
    if (find_handle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    int subdir_count = 0;
    char subdirs[28][MAX_PATH];

    do
    {
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            char* dir_name = find_data.cFileName;

            if (dir_name[0] != '.')
            {
                wb_string_copy(subdirs[subdir_count], path, MAX_PATH);
                strcat(subdirs[subdir_count], "\\");
                strcat(subdirs[subdir_count], dir_name);
                subdir_count++;
            }
        }
        else
        {
            char file_path[MAX_PATH];
            wb_string_copy(file_path, path, MAX_PATH);
            strcat(file_path, "\\");
            strcat(file_path, find_data.cFileName);

            file_proc(file_path);
        }
    }
    while (FindNextFileA(find_handle, &find_data) != 0);

    for (int i = 0; i < subdir_count; i++)
    {
        wb_filesystem_directory_iterate(subdirs[i], file_proc);
    }

    FindClose(find_handle);
}
