#include "gui.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

enum
{
    MAX_WINDOWS = 8,
    MAX_MENUS = 16,
    MAX_TABS = 8
};

struct GuiWindow
{
    HWND handle;
};

struct GuiMenu
{
    HMENU handle;
};

struct GuiTab
{
    HWND handle;
    int item_count;
};

static bool quit;
static HINSTANCE hinstance;
static HANDLE process_heap;

static GuiWindow windows[MAX_WINDOWS];
static GuiMenu menus[MAX_MENUS];
static GuiTab tabs[MAX_TABS];

static LRESULT CALLBACK window_procedure(HWND, UINT, WPARAM, LPARAM);

static bool on_create(HWND handle, LPCREATESTRUCT create);
static void on_destroy(HWND handle);
static void on_command(HWND handle);
static void on_paint(HWND handle);
static void on_size(HWND handle, UINT state, int cx, int cy);

#define find_free_index(index, items, count)    \
    for (int i = 0; i < (count); i++) {         \
        if (!(items)[i].handle) {               \
            (index) = i;                        \
            break;                              \
        }                                       \
    }                                           \
    if ((index) == -1) {                        \
        return NULL;                            \
    }

void gui_initialize(void)
{
    hinstance = GetModuleHandle(NULL);
    process_heap = GetProcessHeap();

    WNDCLASS window_class = { 0 };
    window_class.lpfnWndProc = window_procedure;
    window_class.lpszClassName = "normal";
    RegisterClass(&window_class);
    window_class.style |= CS_DROPSHADOW;
    window_class.lpszClassName = "shadow";
    RegisterClass(&window_class);

    INITCOMMONCONTROLSEX common_controls;
    common_controls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    common_controls.dwICC = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES;
    InitCommonControlsEx(&common_controls);
}

void gui_quit(void)
{
    quit = true;
}

int gui_message_loop(void)
{
    int result;
    MSG message = { 0 };

    while (!quit)
    {
        if (!GetMessage(&message, NULL, 0, 0))
        {
            return message.wParam;
        }

        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return 0;
}

GuiWindow* gui_create_window(GuiWindow* owner, const char* title, int width, int height)
{
    int index = -1;
    find_free_index(index, windows, MAX_WINDOWS);

    GuiWindow* window = &windows[index];

    window->handle = CreateWindow("normal", title, WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, width ? width : CW_USEDEFAULT, height ? height : CW_USEDEFAULT,
                                owner ? owner->handle : NULL, NULL, NULL, NULL);
    if (!window->handle)
    {
        return NULL;
    }

    SetWindowLongPtr(window->handle, GWLP_USERDATA, (LONG_PTR) window);

    ShowWindow(window->handle, SW_SHOW);
    return window;
}

GuiMenu* gui_menu_begin(GuiWindow* window)
{
    int index = -1;
    find_free_index(index, menus, MAX_MENUS);

    GuiMenu* menu = &menus[index];

    menu->handle = CreateMenu();

    return menu;
}

void gui_menu_end(GuiWindow* window, GuiMenu* menu)
{
    SetMenu(window->handle, menu->handle);
}

GuiMenu* gui_menu_add(void)
{
    int index = -1;
    find_free_index(index, menus, MAX_MENUS);

    GuiMenu* menu = &menus[index];

    menu->id = index;
    menu->handle = CreateMenu();
}

void gui_menu_add_item(GuiMenu* menu, const char* name)
{
    AppendMenu(menu->handle, MF_STRING, NULL, name);
}

void gui_add_tab_item(GuiTab* tab, const char* name)
{
    if (!tab || !tab->handle)
        return;

    TCITEM item = { 0 };
    item.mask = TCIF_TEXT;
    item.pszText = (char*) name;
    item.cchTextMax = strlen(name);

    SendMessage(tab->handle, TCM_INSERTITEM, tab->item_count++, (LPARAM) &item);
}

GuiTab* gui_create_tab(GuiWindow* owner)
{
    if (!owner || !owner->handle)
    {
        return NULL;
    }

    int index = -1;
    find_free_index(index, tabs, MAX_TABS);

    GuiTab* tab = &tabs[index];
    tab->item_count = 0;

    RECT client_rect;
    GetClientRect(owner->handle, &client_rect);
    tab->handle = CreateWindowEx(0, WC_TABCONTROL, 0,
                               TCS_FIXEDWIDTH | WS_CHILD | WS_VISIBLE,
                               client_rect.left, client_rect.top + 40, client_rect.right, client_rect.bottom,
                               owner->handle, (HMENU) 8990, hinstance, 0);
    if (!tab->handle)
    {
        return NULL;
    }

    SendMessage(tab->handle, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

    SetWindowLongPtr(owner->handle, GWLP_USERDATA, (LONG_PTR) tab->handle);

    return tab;
}

static LRESULT CALLBACK window_procedure(HWND handle, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(handle, WM_CREATE, on_create);
        HANDLE_MSG(handle, WM_DESTROY, on_destroy);
        HANDLE_MSG(handle, WM_COMMAND, on_command);
        HANDLE_MSG(handle, WM_PAINT, on_paint);
        HANDLE_MSG(handle, WM_SIZE, on_size);
        default:
            return DefWindowProc(handle, uMsg, wParam, lParam);
    }
}

static bool on_create(HWND handle, LPCREATESTRUCT create)
{
    return true;
}

static void on_destroy(HWND handle)
{
    PostQuitMessage(0);
}

static void on_command(HWND handle)
{

}

static void on_paint(HWND handle)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(handle, &ps);
    FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));
    EndPaint(handle, &ps);
}

static void on_size(HWND handle, UINT state, int cx, int cy)
{
    (void) state;
    HWND tab = (HWND) GetWindowLongPtr(handle, GWLP_USERDATA);
    if (!tab)
    {
        return;
    }

    MoveWindow(tab, 2, 2, cx - 4, cy - 4, TRUE);
}