/*
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Copyright (c) Arvid Gerstmann. All rights reserved.
 */
#ifndef _WINDOWS_
#ifndef WINDOWS_GDI_H
#define WINDOWS_GDI_H

#ifndef WINDOWS_BASE_H
#include "windows_base.h"
#endif

/* GDI: */
#define BLACK_BRUSH 4
#define NULL_BRUSH  5

/* Icons / Cursor: */
#define MAKEINTRESOURCE(res)        ((ULONG_PTR)(USHORT)res)
#define IDI_APPLICATION             MAKEINTRESOURCE(32512)
#define IDC_ARROW                   MAKEINTRESOURCE(32512)

/* ========================================================================= */
/* Functions                                                                 */
HCURSOR WINAPI LoadCursorA(
        HINSTANCE hInstance,
        LPCSTR  lpCursorName);
HCURSOR WINAPI LoadCursorW(
        HINSTANCE hInstance,
        LPCWSTR lpCursorName);
HICON WINAPI LoadIconA(
        HINSTANCE hInstance,
        LPCSTR  lpIconName);
HICON WINAPI LoadIconW(
        HINSTANCE hInstance,
        LPCWSTR lpIconName);
HGDIOBJ WINAPI GetStockObject(
        int     fnObject);
HCURSOR WINAPI SetCursor(
        HCURSOR hCursor);

#endif /* WINDOWS_GDI_H */
#endif /* _WINDOWS_ */

