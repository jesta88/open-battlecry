#include "gui.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

enum
{
    MAX_WINDOWS = 8,
    MAX_MENUS = 16,
    MAX_MENU_CHILDREN = 16,
    MAX_MENU_NAME_LENGTH = 32,
    MAX_CONTROLS = 256,
    MAX_TABS = 8,
};

struct GuiWindow
{
    HWND handle;
};

struct GuiMenu
{
    HMENU handle;
    int child_count;
    int children_index[MAX_MENU_CHILDREN];
    char name[MAX_MENU_NAME_LENGTH];
};

struct GuiControl
{
    int id;
    int _pad;
    gui_callback callback;
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
static GuiControl controls[MAX_CONTROLS];
static GuiTab tabs[MAX_TABS];

static LRESULT CALLBACK window_procedure(HWND, UINT, WPARAM, LPARAM);

static bool on_create(HWND handle, LPCREATESTRUCT create);
static void on_destroy(HWND handle);
static void on_command(HWND handle, int id, HWND control, UINT code);
static void on_paint(HWND handle);
static void on_size(HWND handle, UINT state, int cx, int cy);

static inline void string_copy(char* dst, const char* src, int length)
{
    const int* last = (int*) _memccpy(dst, src, 0, length);
    if (!last)
        *(dst + length) = 0;
}

#define find_free_handle(index, items, count)    \
    for (int i = 0; i < (count); i++) {         \
        if (!(items)[i].handle) {               \
            (index) = i;                        \
            break;                              \
        }                                       \
    }                                           \
    if ((index) == -1) {                        \
        return NULL;                            \
    }

#define find_free_index(index, items, count)    \
    for (int i = 0; i < (count); i++) {         \
        if ((items)[i].id == 0) {               \
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
    find_free_handle(index, windows, MAX_WINDOWS);

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

GuiMenu* gui_menu_create(void)
{
    int index = -1;
    find_free_handle(index, menus, MAX_MENUS);

    GuiMenu* menu = &menus[index];

    menu->handle = CreateMenu();

    return menu;
}

GuiMenu* gui_submenu_create(GuiMenu* parent, const char* name)
{
    if (!parent || !parent->handle)
    {
        return NULL;
    }

    int index = -1;
    find_free_handle(index, menus, MAX_MENUS);

    GuiMenu* menu = &menus[index];

    menu->handle = CreateMenu();
    string_copy(menu->name, name, MAX_MENU_NAME_LENGTH);

    parent->children_index[parent->child_count] = index;
    parent->child_count++;

    return menu;
}

static void append_submenus(GuiMenu* menu)
{
    for (int i = 0; i < menu->child_count; i++)
    {
        GuiMenu* submenu = &menus[menu->children_index[i]];
        if (submenu->child_count > 0)
        {
            append_submenus(submenu);
        }

        AppendMenu(menu->handle, MF_POPUP, (UINT_PTR) submenu->handle, submenu->name);
    }
}

void gui_menu_link(GuiMenu* menu, GuiWindow* window)
{
    if (!window || !window->handle || !menu || !menu->handle)
    {
        return;
    }

    if (menu->child_count > 0)
    {
        append_submenus(menu);
    }

    SetMenu(window->handle, menu->handle);
}

GuiControl* gui_menu_add_item(GuiMenu* menu, const char* name, const GuiIcon* icon, gui_callback callback)
{
    if (!menu || !menu->handle)
    {
        return NULL;
    }

    int index = -1;
    find_free_index(index, controls, MAX_CONTROLS);

    GuiControl* control = &controls[index];
    control->id = index + 1;
    control->callback = callback;

    AppendMenu(menu->handle, MF_STRING, control->id, name);

    return control;
}

void gui_tab_add_item(GuiTab* tab, const char* name)
{
    if (!tab || !tab->handle)
        return;

    TCITEM item = { 0 };
    item.mask = TCIF_TEXT;
    item.pszText = (char*) name;
    item.cchTextMax = strlen(name);

    SendMessage(tab->handle, TCM_INSERTITEM, tab->item_count++, (LPARAM) &item);
}

GuiTab* gui_tab_create(GuiWindow* owner)
{
    if (!owner || !owner->handle)
    {
        return NULL;
    }

    int index = -1;
    find_free_handle(index, tabs, MAX_TABS);

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

GuiLabel* gui_label_create(GuiWindow* owner, const char* text)
{
    if (!owner || !owner->handle)
    {
        return NULL;
    }
    
    CreateWindow("static", text, WS_VISIBLE | WS_CHILD, 0, 0, 200, 100, owner->handle, NULL, hinstance, NULL);

    return NULL;
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

static void on_command(HWND window_handle, int id, HWND control_handle, UINT code)
{
    int index = id - 1;
    GuiControl* control = &controls[index];

    if (control->id == 0 || !control->callback)
    {
        return;
    }

    control->callback();
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

    MoveWindow(tab, 0, 0, cx, cy, TRUE);
}