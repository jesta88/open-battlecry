#include <warkit/window.h>
#include <warkit/input.h>
#include "windows_gdi.h"
#include "windows_dpi.h"
#include "windows_misc.h"
#include "windows_window.h"
#include "windows_monitor.h"
#include <math.h>

#define WK_WINDOW_CLASS_NAME "Warkit Window"

typedef struct
{
    HWND handle;
    char* title;
    float scale;
    uint16 width;
    uint16 height;
    RECT windowed_rect;
    bool is_resizing;
    bool quit_requested;
    bool fullscreen;
} wk_window;

static wk_window g_window;

static const uint8 g_keycodes[512] = {
    [0x00B] = WK_BUTTON_0,
    [0x002] = WK_BUTTON_1,
    [0x003] = WK_BUTTON_2,
    [0x004] = WK_BUTTON_3,
    [0x005] = WK_BUTTON_4,
    [0x006] = WK_BUTTON_5,
    [0x007] = WK_BUTTON_6,
    [0x008] = WK_BUTTON_7,
    [0x009] = WK_BUTTON_8,
    [0x00A] = WK_BUTTON_9,
    [0x01E] = WK_BUTTON_A,
    [0x030] = WK_BUTTON_B,
    [0x02E] = WK_BUTTON_C,
    [0x020] = WK_BUTTON_D,
    [0x012] = WK_BUTTON_E,
    [0x021] = WK_BUTTON_F,
    [0x022] = WK_BUTTON_G,
    [0x023] = WK_BUTTON_H,
    [0x017] = WK_BUTTON_I,
    [0x024] = WK_BUTTON_J,
    [0x025] = WK_BUTTON_K,
    [0x026] = WK_BUTTON_L,
    [0x032] = WK_BUTTON_M,
    [0x031] = WK_BUTTON_N,
    [0x018] = WK_BUTTON_O,
    [0x019] = WK_BUTTON_P,
    [0x010] = WK_BUTTON_Q,
    [0x013] = WK_BUTTON_R,
    [0x01F] = WK_BUTTON_S,
    [0x014] = WK_BUTTON_T,
    [0x016] = WK_BUTTON_U,
    [0x02F] = WK_BUTTON_V,
    [0x011] = WK_BUTTON_W,
    [0x02D] = WK_BUTTON_X,
    [0x015] = WK_BUTTON_Y,
    [0x02C] = WK_BUTTON_Z,

    [0x028] = WK_BUTTON_APOSTROPHE,
    [0x02B] = WK_BUTTON_BACKSLASH,
    [0x033] = WK_BUTTON_COMMA,
    [0x00D] = WK_BUTTON_EQUAL,
    [0x029] = WK_BUTTON_GRAVE_ACCENT,
    [0x01A] = WK_BUTTON_LEFT_BRACKET,
    [0x00C] = WK_BUTTON_DASH,
    [0x034] = WK_BUTTON_PERIOD,
    [0x01B] = WK_BUTTON_RIGHT_BRACKET,
    [0x027] = WK_BUTTON_SEMICOLON,
    [0x035] = WK_BUTTON_SLASH,
    [0x056] = WK_BUTTON_WORLD_2,

    [0x029] = WK_BUTTON_CIRCUMFLEX,
    [0x00E] = WK_BUTTON_BACKSPACE,
    [0x153] = WK_BUTTON_DELETE,
    [0x14F] = WK_BUTTON_END,
    [0x01C] = WK_BUTTON_ENTER,
    [0x001] = WK_BUTTON_ESCAPE,
    [0x147] = WK_BUTTON_HOME,
    [0x152] = WK_BUTTON_INSERT,
    [0x15D] = WK_BUTTON_MENU,
    [0x151] = WK_BUTTON_PAGE_DOWN,
    [0x149] = WK_BUTTON_PAGE_UP,
    [0x045] = WK_BUTTON_PAUSE,
    [0x146] = WK_BUTTON_PAUSE,
    [0x039] = WK_BUTTON_SPACE,
    [0x00F] = WK_BUTTON_TAB,
    [0x03A] = WK_BUTTON_CAPS_LOCK,
    [0x145] = WK_BUTTON_NUM_LOCK,
    [0x046] = WK_BUTTON_SCROLL_LOCK,
    [0x03B] = WK_BUTTON_F1,
    [0x03C] = WK_BUTTON_F2,
    [0x03D] = WK_BUTTON_F3,
    [0x03E] = WK_BUTTON_F4,
    [0x03F] = WK_BUTTON_F5,
    [0x040] = WK_BUTTON_F6,
    [0x041] = WK_BUTTON_F7,
    [0x042] = WK_BUTTON_F8,
    [0x043] = WK_BUTTON_F9,
    [0x044] = WK_BUTTON_F10,
    [0x057] = WK_BUTTON_F11,
    [0x058] = WK_BUTTON_F12,
    [0x064] = WK_BUTTON_F13,
    [0x065] = WK_BUTTON_F14,
    [0x066] = WK_BUTTON_F15,
    [0x067] = WK_BUTTON_F16,
    [0x068] = WK_BUTTON_F17,
    [0x069] = WK_BUTTON_F18,
    [0x06A] = WK_BUTTON_F19,
    [0x06B] = WK_BUTTON_F20,
    [0x06C] = WK_BUTTON_F21,
    [0x06D] = WK_BUTTON_F22,
    [0x06E] = WK_BUTTON_F23,
    [0x076] = WK_BUTTON_F24,
    [0x038] = WK_BUTTON_LEFT_ALT,
    [0x01D] = WK_BUTTON_LEFT_CONTROL,
    [0x02A] = WK_BUTTON_LEFT_SHIFT,
    [0x15B] = WK_BUTTON_LEFT_SUPER,
    [0x137] = WK_BUTTON_PRINT_SCREEN,
    [0x138] = WK_BUTTON_RIGHT_ALT,
    [0x11D] = WK_BUTTON_RIGHT_CONTROL,
    [0x036] = WK_BUTTON_RIGHT_SHIFT,
    [0x15C] = WK_BUTTON_RIGHT_SUPER,
    [0x150] = WK_BUTTON_DOWN,
    [0x14B] = WK_BUTTON_LEFT,
    [0x14D] = WK_BUTTON_RIGHT,
    [0x148] = WK_BUTTON_UP,

    [0x052] = WK_BUTTON_NUM_0,
    [0x04F] = WK_BUTTON_NUM_1,
    [0x050] = WK_BUTTON_NUM_2,
    [0x051] = WK_BUTTON_NUM_3,
    [0x04B] = WK_BUTTON_NUM_4,
    [0x04C] = WK_BUTTON_NUM_5,
    [0x04D] = WK_BUTTON_NUM_6,
    [0x047] = WK_BUTTON_NUM_7,
    [0x048] = WK_BUTTON_NUM_8,
    [0x049] = WK_BUTTON_NUM_9,
    [0x04E] = WK_BUTTON_NUM_ADD,
    [0x053] = WK_BUTTON_NUM_DECIMAL,
    [0x135] = WK_BUTTON_NUM_DIVIDE,
    [0x11C] = WK_BUTTON_NUM_ENTER,
    [0x037] = WK_BUTTON_NUM_MULTIPLY,
    [0x04A] = WK_BUTTON_NUM_SUBTRACT};

static LRESULT WINAPI window_callback(HWND, UINT, WPARAM, LPARAM);

int wk_open_window(const wk_window_desc* window_desc)
{
    const HINSTANCE instance = wk_get_win32_hinstance();

    // Window class
    WNDCLASSEXA window_class = {
        .cbSize = sizeof(window_class),
        .style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = window_callback,
        .hInstance = instance,
        .hIcon = LoadIconA(NULL, (LPCSTR)IDI_APPLICATION),
        .hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW),
        .hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH),
        .lpszClassName = WK_WINDOW_CLASS_NAME};
    if (!RegisterClassExA(&window_class)) {
        wk_log_error("Failed to register window class with error: %d", GetLastError());
        return 1;
    }

    // Window
    DWORD style = WS_POPUP;
    if (!window_desc->fullscreen)
        style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER;

    HWND hwnd = CreateWindowExA(
        WS_EX_APPWINDOW,      // dwExStyle
        WK_WINDOW_CLASS_NAME, // lpClassName
        window_desc->title,   // lpWindowName
        style,                // dwStyle
        CW_USEDEFAULT,        // X
        CW_USEDEFAULT,        // Y
        window_desc->width,   // nWidth
        window_desc->height,  // nHeight
        NULL,                 // hWndParent
        NULL,                 // hMenu
        instance,             // hInstance
        NULL);                // lParam
    if (hwnd == NULL) {
        wk_log_error("Failed to create window with error: %d", GetLastError());
        return 1;
    }
    g_window.handle = hwnd;

    return 0;
}

static uint16 translate_key(WPARAM wparam, LPARAM lparam)
{
    if (wparam == VK_CONTROL) {
        if (lparam & 0x1000000)
            return WK_BUTTON_RIGHT_CONTROL;
        return WK_BUTTON_LEFT_CONTROL;
    }
    if (wparam == VK_PROCESSKEY) {
        return WK_BUTTON_NONE;
    }

    return g_keycodes[(lparam >> 16) & 0x1FF];
}

static uint8 get_modifier(void)
{
    uint8 mod = 0;
    if (GetKeyState(VK_SHIFT) & (1 << 31))
        mod |= WK_BUTTON_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & (1 << 31))
        mod |= WK_BUTTON_MOD_CONTROL;
    if (GetKeyState(VK_MENU) & (1 << 31))
        mod |= WK_BUTTON_MOD_ALT;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & (1 << 31))
        mod |= WK_BUTTON_MOD_SUPER;
    return mod;
}

static void process_keys(wk_event* event, WPARAM wparam, LPARAM lparam)
{
    uint16 key = translate_key(wparam, lparam);
    uint16 scancode = (lparam >> 16) & 0x1FF;
    bool pressed = !((lparam >> 31) & 0x1);
    uint8 modifier = get_modifier();

    event->key.keycode = key;
    event->key.scancode = scancode << 5;
    event->key.modifiers = modifier << 1;
    event->key.pressed = pressed;
}

bool wk_poll_events(wk_event* event)
{
    MSG msg;
    if (!PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        return true;
    }

    if (msg.message == WM_QUIT) {
        return false;
    }

    TranslateMessage(&msg);
    DispatchMessageA(&msg);
    return true;
}

void wk_show_window(void)
{
    ShowWindow(g_window.handle, SW_SHOW);
    UpdateWindow(g_window.handle);
}

void wk_close_window(void)
{
    DestroyWindow(g_window.handle);
    UnregisterClassA(WK_WINDOW_CLASS_NAME, wk_get_win32_hinstance());
}

void* wk_get_win32_hwnd(void)
{
    return g_window.handle;
}

static void window_resized(int width, int height)
{
    wk_event event;
    event.type = WK_WINDOW_EVENT_RESIZED;
    event.resize.width = (uint16)width;
    event.resize.height = (uint16)height;

    // TODO: Call engine event callback
}

static LRESULT WINAPI window_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
        const bool activated = LOWORD(wparam) > 0;
        const bool has_focus = activated && !IsIconic(hwnd);
        // TODO: toggle_fullscreen(has_focus);
        break;
    case WM_CLOSE:
        DestroyWindow(g_window.handle);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_ENTERSIZEMOVE:
        g_window.is_resizing = true;
        return 0;
    case WM_EXITSIZEMOVE:
        if (!g_window.is_resizing)
            break;

        g_window.is_resizing = false;

        window_resized(g_window.width, g_window.height);
        return 0;
    case WM_SIZE:
        const uint16 width = LOWORD(lparam);
        const uint16 height = HIWORD(lparam);

        // Minimized or maximized
        if (wparam == 2 || wparam == 1 || (wparam == 0 && !g_window.is_resizing)) {
            g_window.is_resizing = false;
            g_window.width = width;
            g_window.height = height;
            window_resized(width, height);
        }

        if (g_window.is_resizing) {
            g_window.width = width;
            g_window.height = height;
        }
        return 0;
    case WM_MOVE:
        return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        wk_event event = {0};
        event.type = WK_WINDOW_EVENT_KEY;
        process_keys(&event, wparam, lparam);
        return true;
    default:
        return DefWindowProcA(hwnd, msg, wparam, lparam);
    }

    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static int update_dimensions(void)
{
    RECT rect;
    if (!GetClientRect(g_window.handle, &rect)) {
        MessageBoxA(NULL, TEXT("GetClientRect failed"), TEXT("Error"), MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    float window_width = (float)(rect.right - rect.left) / g_window.scale;
    float window_height = (float)(rect.bottom - rect.top) / g_window.scale;
    g_window.width = (uint16)roundf(window_width);
    g_window.height = (uint16)roundf(window_height);
    return false;
}

static void set_fullscreen(bool fullscreen, UINT swp_flags)
{
    HMONITOR monitor = MonitorFromWindow(g_window.handle, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitor_info = {0};
    monitor_info.cbSize = sizeof(MONITORINFO);
    GetMonitorInfoA(monitor, &monitor_info);
    const RECT monitor_rect = monitor_info.rcMonitor;
    const int monitor_width = monitor_rect.right - monitor_rect.left;
    const int monitor_height = monitor_rect.bottom - monitor_rect.top;

    const DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    DWORD style;
    RECT rect = {0, 0, 0, 0};

    g_window.fullscreen = fullscreen;
    if (!g_window.fullscreen) {
        style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX;
        rect = g_window.windowed_rect;
    } else {
        GetWindowRect(g_window.handle, &g_window.windowed_rect);
        style = WS_POPUP | WS_SYSMENU | WS_VISIBLE;
        rect.left = monitor_rect.left;
        rect.top = monitor_rect.top;
        rect.right = rect.left + monitor_width;
        rect.bottom = rect.top + monitor_height;
        AdjustWindowRectEx(&rect, style, FALSE, ex_style);
    }
    const int win_w = rect.right - rect.left;
    const int win_h = rect.bottom - rect.top;
    const int win_x = rect.left;
    const int win_y = rect.top;
    SetWindowLongPtrA(g_window.handle, GWL_STYLE, style);
    SetWindowPos(g_window.handle, HWND_TOP, win_x, win_y, win_w, win_h, swp_flags | SWP_FRAMECHANGED);
}

static void toggle_fullscreen(void)
{
    set_fullscreen(!g_window.fullscreen, SWP_SHOWWINDOW);
}

static void set_dpi_awareness(bool high_dpi)
{
    if (high_dpi) {
        DPI_AWARENESS_CONTEXT per_monitor_aware_v2 = (DPI_AWARENESS_CONTEXT)-4;
        if (!SetProcessDpiAwarenessContext(per_monitor_aware_v2)) {
            MessageBoxA(NULL, TEXT("Could not set process DPI awareness v2"), TEXT("Error"), MB_ICONEXCLAMATION | MB_OK);
        }
        POINT pt = {1, 1};
        HMONITOR hm = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
        UINT dpix, dpiy;
        HRESULT hr = GetDpiForMonitor(hm, MDT_EFFECTIVE_DPI, &dpix, &dpiy);
        if (!SUCCEEDED(hr)) {
            MessageBoxA(NULL, TEXT("Could not get DPI for monitor"), TEXT("Error"), MB_ICONEXCLAMATION | MB_OK);
        }
        g_window.scale = (float)dpix / 96.0f;
    }
}