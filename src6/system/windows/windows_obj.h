/*
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Copyright (c) Arvid Gerstmann. All rights reserved.
 */
#ifndef _WINDOWS_
#ifndef WINDOWS_WINDOW_H
#define WINDOWS_WINDOW_H

/* Disable all warnings */
#if defined(_MSC_VER)
    #pragma warning(push, 0)
#endif

#ifndef WINDOWS_BASE_H
#include "windows_base.h"
#endif
#if defined(__cplusplus)
extern "C" {
#endif

    HRESULT WINAPI CoInitializeEx(
        LPVOID pvReserved,
        DWORD  dwCoInit
    );
    void WINAPI CoUninitialize();
    PIDLIST_ABSOLUTE WINAPI SHBrowseForFolderA(
        LPBROWSEINFOA lpbi
    );
    BOOL WINAPI SHGetPathFromIDListA(
        PCIDLIST_ABSOLUTE pidl,
        LPSTR             pszPath
    );

#if defined(__cplusplus)
}
#endif

/* Enable all warnings */
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif /* WINDOWS_WINDOW_H */
#endif /* _WINDOWS_ */
