#ifndef _WINDOWS_
#ifndef WINDOWS_MONITOR_H
#define WINDOWS_MONITOR_H

#ifndef WINDOWS_BASE_H
#include "windows_base.h"
#endif

#define GWL_STYLE   (-16)
#define GWL_EXSTYLE (-20)

/*
 * SetWindowPos Flags
 */
#define SWP_NOSIZE         0x0001
#define SWP_NOMOVE         0x0002
#define SWP_NOZORDER       0x0004
#define SWP_NOREDRAW       0x0008
#define SWP_NOACTIVATE     0x0010
#define SWP_FRAMECHANGED   0x0020 /* The frame changed: send WM_NCCALCSIZE */
#define SWP_SHOWWINDOW     0x0040
#define SWP_HIDEWINDOW     0x0080
#define SWP_NOCOPYBITS     0x0100
#define SWP_NOOWNERZORDER  0x0200 /* Don't do owner Z ordering */
#define SWP_NOSENDCHANGING 0x0400 /* Don't send WM_WINDOWPOSCHANGING */

#define SWP_DRAWFRAME    SWP_FRAMECHANGED
#define SWP_NOREPOSITION SWP_NOOWNERZORDER

#define SWP_DEFERERASE     0x2000 // same as SWP_DEFERDRAWING
#define SWP_ASYNCWINDOWPOS 0x4000 // same as SWP_CREATESPB

#define HWND_TOP       ((HWND)0)
#define HWND_BOTTOM    ((HWND)1)
#define HWND_TOPMOST   ((HWND)-1)
#define HWND_NOTOPMOST ((HWND)-2)

#define MONITOR_DEFAULTTONULL    0x00000000
#define MONITOR_DEFAULTTOPRIMARY 0x00000001
#define MONITOR_DEFAULTTONEAREST 0x00000002

#define MONITORINFOF_PRIMARY 0x00000001

#ifndef CCHDEVICENAME
#define CCHDEVICENAME 32
#endif

DECLARE_HANDLE(HMONITOR);

typedef struct tagMONITORINFO
{
    DWORD cbSize;
    RECT rcMonitor;
    RECT rcWork;
    DWORD dwFlags;
} MONITORINFO, *LPMONITORINFO;

typedef struct tagMONITORINFOEXA
{
    MONITORINFO DUMMYSTRUCTNAME;
    CHAR szDevice[CCHDEVICENAME];
} MONITORINFOEXA, *LPMONITORINFOEXA;
typedef struct tagMONITORINFOEXW
{
    MONITORINFO DUMMYSTRUCTNAME;
    WCHAR szDevice[CCHDEVICENAME];
} MONITORINFOEXW, *LPMONITORINFOEXW;

HMONITOR WINAPI MonitorFromPoint(
    POINT pt,
    DWORD dwFlags);

HMONITOR WINAPI MonitorFromRect(
    LPCRECT lprc,
    DWORD dwFlags);

HMONITOR WINAPI MonitorFromWindow(
    HWND hwnd,
    DWORD dwFlags);

BOOL WINAPI GetMonitorInfoA(
        HMONITOR hMonitor,
        LPMONITORINFO lpmi);
BOOL WINAPI GetMonitorInfoW(
        HMONITOR hMonitor,
        LPMONITORINFO lpmi);

typedef BOOL(CALLBACK* MONITORENUMPROC)(HMONITOR, HDC, LPRECT, LPARAM);

BOOL WINAPI EnumDisplayMonitors(
        HDC hdc,
        LPCRECT lprcClip,
        MONITORENUMPROC lpfnEnum,
        LPARAM dwData);

/* ========================================================================== */
/* Shellscalingapi: */
typedef enum MONITOR_DPI_TYPE
{
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI = 1,
    MDT_RAW_DPI = 2,
    MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;

HRESULT WINAPI GetDpiForMonitor(
    HMONITOR hmonitor,
    MONITOR_DPI_TYPE dpiType,
    UINT* dpiX,
    UINT* dpiY);

#endif /* WINDOWS_MONITOR_H */
#endif /* _WINDOWS_ */