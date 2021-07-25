#include "gui.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR pCmdLine, _In_ int nCmdShow)
{
    //int screen_width = GetSystemMetrics(SM_CXSCREEN);
    //int screen_height = GetSystemMetrics(SM_CYSCREEN);

    //char directory[MAX_PATH];
    //GetCurrentDirectory(MAX_PATH, directory);

    gui_initialize();

    GuiWindow* window = gui_create_window(NULL, "Open Battlecry Editor", 0, 0);

    GuiMenu* menu = gui_menu_begin(window);
    gui_menu_add_item(menu, "File");
    gui_menu_end(window, menu);

    GuiTab* tab = gui_create_tab(window);
    gui_add_tab_item(tab, "Text");
    gui_add_tab_item(tab, "GUI");
    gui_add_tab_item(tab, "Unit");
    gui_add_tab_item(tab, "Building");
    gui_add_tab_item(tab, "Hero");
    gui_add_tab_item(tab, "Item");

    return gui_message_loop();
}
