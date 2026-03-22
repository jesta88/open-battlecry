#include "include/carbon_window.h"
#include "win32_common.h"
#include <assert.h>
#include <math.h>

#define CS_OWNDC 0x0020
#define CS_VREDRAW 0x0001
#define CS_HREDRAW 0x0002
#define MAKEINTRESOURCE(res) ((ULONG_PTR)(USHORT)res)
#define IDI_APPLICATION MAKEINTRESOURCE(32512)
#define IDC_ARROW MAKEINTRESOURCE(32512)
#define BLACK_BRUSH 4
#define WS_OVERLAPPED 0x00000000L
#define WS_CAPTION 0x00C00000L
#define WS_SYSMENU 0x00080000L
#define WS_THICKFRAME 0x00040000L
#define WS_MINIMIZE 0x20000000L
#define WS_MINIMIZEBOX 0x00020000L
#define WS_MAXIMIZEBOX 0x00010000L
#define WS_POPUP 0x80000000L
#define WS_CLIPCHILDREN 0x02000000L
#define WS_CLIPSIBLINGS 0x04000000L
#define WS_VISIBLE 0x10000000L
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED     | \
                             WS_CAPTION        | \
                             WS_SYSMENU        | \
                             WS_THICKFRAME     | \
                             WS_MINIMIZEBOX    | \
                             WS_MAXIMIZEBOX)
#define WS_EX_APPWINDOW 0x00040000L
#define CW_USEDEFAULT ((int)0x80000000)
#define MONITOR_DEFAULTTOPRIMARY 0x00000001
#define CCHDEVICENAME 32
#define SW_SHOW 5
#define SW_MAXIMIZE 3
#define SW_NORMAL 1
#define PM_REMOVE 0x0001
#define WM_DESTROY 0x0002
#define WM_QUIT 0x0012
#define WM_NCPAINT 0x0085
#define WM_WINDOWPOSCHANGED 0x0047
#define WM_STYLECHANGED 0x007D
#define WM_DISPLAYCHANGE 0x007E
#define WM_GETMINMAXINFO 0x0024
#define WM_CLOSE 0x0010
#define WM_ERASEBKGND 0x0014
#define GWL_STYLE (-16)
#define HWND_NOTOPMOST ((HWND)-2)
#define SWP_FRAMECHANGED 0x0020
#define SWP_NOACTIVATE 0x0010
#define SWP_NOOWNERZORDER 0x0200

typedef LRESULT (CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct tagPOINT {
    LONG x;
    LONG y;
} POINT;
typedef struct _RECT {
    LONG        left;
    LONG        top;
    LONG        right;
    LONG        bottom;
} RECT, *LPRECT;
typedef struct tagWNDCLASSEX {
    UINT      cbSize;
    UINT      style;
    WNDPROC   lpfnWndProc;
    int       cbClsExtra;
    int       cbWndExtra;
    HINSTANCE hInstance;
    HICON     hIcon;
    HCURSOR   hCursor;
    HBRUSH    hbrBackground;
    LPCSTR    lpszMenuName;
    LPCSTR    lpszClassName;
    HICON     hIconSm;
} WNDCLASSEX;
typedef struct tagMONITORINFO
{
    DWORD   cbSize;
    RECT    rcMonitor;
    RECT    rcWork;
    DWORD   dwFlags;
} MONITORINFO, * LPMONITORINFO;
typedef struct tagMONITORINFOEXA
{
    MONITORINFO DUMMYSTRUCTNAME;
    CHAR        szDevice[CCHDEVICENAME];
} MONITORINFOEXA;
typedef struct tagMSG {
    HWND        hwnd;
    UINT        message;
    WPARAM      wParam;
    LPARAM      lParam;
    DWORD       time;
    POINT       pt;
} MSG, *PMSG, *LPMSG;
typedef struct tagMINMAXINFO {
    POINT ptReserved;
    POINT ptMaxSize;
    POINT ptMaxPosition;
    POINT ptMinTrackSize;
    POINT ptMaxTrackSize;
} MINMAXINFO, * PMINMAXINFO, * LPMINMAXINFO;

ATOM RegisterClassExA(
    const WNDCLASSEX* unnamedParam1);
HMODULE WINAPI GetModuleHandleA(
    LPCSTR lpModuleName);
HCURSOR WINAPI LoadCursorA(
    HINSTANCE hInstance,
    LPCSTR  lpCursorName);
HICON WINAPI LoadIconA(
    HINSTANCE hInstance,
    LPCSTR  lpIconName);
HGDIOBJ WINAPI GetStockObject(
    int     fnObject);
BOOL WINAPI AdjustWindowRect(
    LPRECT lpRect,
    DWORD dwStyle,
    BOOL bMenu);
HWND WINAPI CreateWindowExA(
    DWORD   dwExStyle,
    LPCSTR  lpClassName,
    LPCSTR  lpWindowName,
    DWORD   dwStyle,
    int     x,
    int     y,
    int     nWidth,
    int     nHeight,
    HWND    hWndParent,
    HMENU   hMenu,
    HINSTANCE hInstance,
    LPVOID  lpParam);
HMONITOR WINAPI MonitorFromWindow(
    HWND hwnd,
    DWORD dwFlags);
BOOL WINAPI GetMonitorInfoA(
    HMONITOR hMonitor,
    LPMONITORINFO lpmi);
BOOL WINAPI ShowWindow(
    HWND    hWnd,
    int     nCmdShow);
BOOL WINAPI PeekMessageA(
    LPMSG   lpMsg,
    HWND    hWnd,
    UINT    wMsgFilterMin,
    UINT    wMsgFilterMax,
    UINT    wRemoveMsg);
BOOL WINAPI TranslateMessage(
    const MSG *lpMsg);
LRESULT WINAPI DispatchMessageA(
    const MSG *lpMsg);
BOOL WINAPI GetWindowRect(
    HWND hWnd,
    LPRECT lpRect);
LONG WINAPI SetWindowLongA(
    HWND hWnd,
    int nIndex,
    LONG dwNewLong);
BOOL WINAPI SetWindowPos(
    HWND hWnd,
    HWND hWndInsertAfter,
    int X,
    int Y,
    int cx,
    int cy,
    UINT uFlags);
LRESULT WINAPI DefWindowProcA(
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam);
HBRUSH  WINAPI CreateSolidBrush(
    COLORREF color);
BOOL WINAPI GetClientRect(
    HWND    hWnd,
    LPRECT  lpRect);
int WINAPI FillRect(
    HDC hDC,
    const RECT* lprc,
    HBRUSH hbr);
void WINAPI PostQuitMessage(
    int nExitCode);
BOOL WINAPI AdjustWindowRectEx(
    LPRECT  lpRect,
    DWORD   dwStyle,
    BOOL    bMenu,
    DWORD   dwExStyle);
BOOL WINAPI AdjustWindowRectExForDpi(
    LPRECT lpRect,
    DWORD  dwStyle,
    BOOL   bMenu,
    DWORD  dwExStyle,
    UINT   dpi
);

typedef struct
{
    HWND handle;
    HICON big_icon;
    HICON small_icon;


} c_window_win32;

struct c_window
{
    const char* title;
    int x, y;
    int width, height;
    int min_width, min_height;
    int max_width, max_height;
    int windowed_x, windowed_y;
    int windowed_width, windowed_height;
    uint32_t flags;
    uint32_t last_fullscreen_flags;
    int monitor_index;
    c_display_mode fullscreen_display_mode;
    bool is_hiding;
    bool is_destroying;
    bool is_dropping;
    uint32_t id;
    uint32_t prev_window_id;
    uint32_t next_window_id;
};

static const char* k_window_class = "carbon";

static DWORD getWindowStyle(const c_window* window)
{
    DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (window->monitor_index != -1)
        style |= WS_POPUP;
    else
    {
        style |= WS_SYSMENU | WS_MINIMIZEBOX;

        if ((window->flags & C_WINDOW_FULLSCREEN) == 0)
        {
            style |= WS_CAPTION;

            if ((window->flags & C_WINDOW_RESIZABLE) != 0)
                style |= WS_MAXIMIZEBOX | WS_THICKFRAME;
        }
        else
            style |= WS_POPUP;
    }

    return style;
}

static DWORD getWindowExStyle(const c_window* window)
{
    return WS_EX_APPWINDOW;
}

static const c_image* chooseImage(int count, const c_image* images, int width, int height)
{
    int i, leastDiff = INT32_MAX;
    const c_image* closest = NULL;

    for (i = 0;  i < count;  i++)
    {
        const int currDiff = abs(images[i].width * images[i].height - width * height);
        if (currDiff < leastDiff)
        {
            closest = images + i;
            leastDiff = currDiff;
        }
    }

    return closest;
}

static void getFullWindowSize(DWORD style, DWORD exStyle,
                              int contentWidth, int contentHeight,
                              int* fullWidth, int* fullHeight,
                              UINT dpi)
{
    RECT rect = { 0, 0, contentWidth, contentHeight };

    if (_c_windows_version_1607_or_greater())
        AdjustWindowRectExForDpi(&rect, style, FALSE, exStyle, dpi);
    else
        AdjustWindowRectEx(&rect, style, FALSE, exStyle);

    *fullWidth = rect.right - rect.left;
    *fullHeight = rect.bottom - rect.top;
}

static void applyAspectRatio(c_window* window, int edge, RECT* area)
{
    int xoff, yoff;
    UINT dpi = USER_DEFAULT_SCREEN_DPI;
    const float ratio = (float) window->numer / (float) window->denom;

    if (_c_windows_version_1607_or_greater())
        dpi = GetDpiForWindow(window->win32.handle);

    getFullWindowSize(getWindowStyle(window), getWindowExStyle(window),
                      0, 0, &xoff, &yoff, dpi);

    if (edge == WMSZ_LEFT  || edge == WMSZ_BOTTOMLEFT ||
        edge == WMSZ_RIGHT || edge == WMSZ_BOTTOMRIGHT)
    {
        area->bottom = area->top + yoff +
                       (int) ((area->right - area->left - xoff) / ratio);
    }
    else if (edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT)
    {
        area->top = area->bottom - yoff -
                    (int) ((area->right - area->left - xoff) / ratio);
    }
    else if (edge == WMSZ_TOP || edge == WMSZ_BOTTOM)
    {
        area->right = area->left + xoff +
                      (int) ((area->bottom - area->top - yoff) * ratio);
    }
}

static LRESULT CALLBACK window_procedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

c_window* c_create_window(const char* title, int width, int height, c_monitor* monitor, c_window_flags flags)
{
    HINSTANCE hinstance = GetModuleHandleA(NULL);
    WNDCLASSEX window_class = {
        .cbSize = sizeof(window_class),
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = window_procedure,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = hinstance,
        .hIcon = LoadIconA(NULL, (LPCSTR)IDI_APPLICATION),
        .hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW),
        .hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH),
        .lpszMenuName = NULL,
        .lpszClassName = k_window_class,
        .hIconSm = 0 };
    RegisterClassExA(&window_class);

    int window_style = WS_OVERLAPPEDWINDOW;
    RECT window_rect = {
        .left = 0,
        .top = 0,
        .right = width,
        .bottom = height,
    };
    AdjustWindowRect(&window_rect, window_style, FALSE);

    HWND hwnd = CreateWindowExA(
        0,
        k_window_class,
        title,
        window_style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top,
        0,
        NULL,
        hinstance,
        NULL);
    assert(hwnd != NULL);
}

static void get_fullscreen_resolution(uint32_t* width, uint32_t* height)
{
    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO info = {
        .cbSize = sizeof(MONITORINFOEXA),
    };
    GetMonitorInfoA(monitor, &info);

    *width = info.rcMonitor.right - info.rcMonitor.left;
    *height = info.rcMonitor.bottom - info.rcMonitor.top;
}

static void show_window(void)
{
    ShowWindow(hwnd, SW_SHOW);
}

static void process_events()
{
    MSG msg = { 0 };
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);

        if (msg.message == WM_QUIT)
        {
            quit_requested = true;
        }
    }
}

static void adjust_window()
{
    if (fullscreen)
    {
        // Save the old window rect so we can restore it when exiting fullscreen mode.
        RECT windowed_rect;
        GetWindowRect(hwnd, &windowed_rect);
        windowed_width = windowed_rect.right - windowed_rect.left;
        windowed_height = windowed_rect.bottom - windowed_rect.top;

        // Make the window borderless so that the client area can fill the screen.
        SetWindowLongA(hwnd, GWL_STYLE, WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);

        // Get the settings of the durrent display index. We want the app to go into
        // fullscreen mode on the display that supports Independent Flip.
        HMONITOR currentMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO info = {
            .cbSize = sizeof(MONITORINFOEXA),
        };
        GetMonitorInfoA(currentMonitor, &info);

        window.x = info.rcMonitor.left;
        window.y = info.rcMonitor.top;

        int window_width = info.rcMonitor.right - info.rcMonitor.left;
        int window_height = info.rcMonitor.bottom - info.rcMonitor.top;
        SetWindowPos(hwnd, HWND_NOTOPMOST, info.rcMonitor.left, info.rcMonitor.top, window_width, window_height, SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ShowWindow(hwnd, SW_MAXIMIZE);

        width = window_width;
        height = window_height;
    }
    else
    {
        DWORD windowStyle = WS_OVERLAPPEDWINDOW;
        SetWindowLongA(hwnd, GWL_STYLE, windowStyle);

        SetWindowPos(
            hwnd, HWND_NOTOPMOST, window.x, window.y, window.width, window.height,
            SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

        if (maximized)
        {
            ShowWindow(hwnd, SW_MAXIMIZE);
        }
        else
        {
            ShowWindow(hwnd, SW_NORMAL);
        }
    }
}

static LRESULT CALLBACK window_procedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_NCPAINT:
        case WM_WINDOWPOSCHANGED:
        case WM_STYLECHANGED:
        {
            return DefWindowProcA(hwnd, message, wParam, lParam);
        }
        case WM_DISPLAYCHANGE:
        {
            adjust_window();
            break;
        }
        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;

            // Prevent window from collapsing
            if (!fullscreen)
            {
                LONG zoom_offset = 128;
                lpMMI->ptMinTrackSize.x = zoom_offset;
                lpMMI->ptMinTrackSize.y = zoom_offset;
            }
            break;
        }
        case WM_ERASEBKGND:
        {
            // Make sure to keep consistent background color when resizing.
            HDC hdc = (HDC) wParam;
            RECT rc;
            HBRUSH hbrWhite = CreateSolidBrush(0x00000000);
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, hbrWhite);
            break;
        }
        case WM_CLOSE:
        {
            PostQuitMessage(0);
            break;
        }
        case WM_DESTROY:
            break;
        default:
            return DefWindowProcA(hwnd, message, wParam, lParam);
    }
    return 0;
}