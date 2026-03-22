#pragma once

#include "types.h"

#define TRUE (1)
#define FALSE (0)
#define WINAPI __stdcall
#define CALLBACK __stdcall

typedef int BOOL;
typedef char CHAR;
typedef wchar_t WCHAR;
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef long LONG;
typedef unsigned long DWORD;
typedef uint64_t UINT_PTR;
typedef uint64_t ULONG_PTR;
typedef int64_t LONG_PTR;
typedef void* LPVOID;
typedef CHAR* LPSTR;
typedef const CHAR* LPCSTR;
typedef WORD ATOM;
typedef LONG_PTR LRESULT;
typedef LONG_PTR LPARAM;
typedef UINT_PTR WPARAM;
typedef LPVOID HANDLE;
typedef HANDLE HINSTANCE;
typedef HANDLE HMODULE;
typedef HANDLE HWND;
typedef HANDLE HMENU;
typedef HANDLE HICON;
typedef HANDLE HBRUSH;
typedef HANDLE HMONITOR;
typedef HANDLE HDC;
typedef HICON HCURSOR;
typedef void* HGDIOBJ;
typedef DWORD COLORREF;

WCHAR* _c_utf8_to_wide(const char* source);
char* _c_wide_to_utf8(const WCHAR* source);
BOOL _c_windows_version_equal_or_greater(WORD major, WORD minor, WORD sp);
BOOL _c_windows_build_equal_or_greater(WORD build);

#define _c_windows_vista_or_greater()                                     \
    _c_windows_version_equal_or_greater(HIBYTE(_WIN32_WINNT_VISTA),   \
                                        LOBYTE(_WIN32_WINNT_VISTA), 0)
#define _c_windows_7_or_greater()                                         \
    _c_windows_version_equal_or_greater(HIBYTE(_WIN32_WINNT_WIN7),    \
                                        LOBYTE(_WIN32_WINNT_WIN7), 0)
#define _c_windows_8_or_greater()                                         \
    _c_windows_version_equal_or_greater(HIBYTE(_WIN32_WINNT_WIN8),    \
                                        LOBYTE(_WIN32_WINNT_WIN8), 0)
#define _c_windows_8_1_or_greater()                                   \
    _c_windows_version_equal_or_greater(HIBYTE(_WIN32_WINNT_WINBLUE), \
                                        LOBYTE(_WIN32_WINNT_WINBLUE), 0)

// Windows 10 Anniversary Update
#define _c_windows_version_1607_or_greater() \
    _c_windows_build_equal_or_greater(14393)
// Windows 10 Creators Update
#define _c_windows_version_1703_or_greater() \
    _c_windows_build_equal_or_greater(15063)
