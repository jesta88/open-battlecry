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

int GetClassNameA(
    HWND  hWnd,
    LPSTR lpClassName,
    int   nMaxCount
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
