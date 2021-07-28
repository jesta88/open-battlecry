#include "gui.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static void quit_callback(void)
{
    gui_quit();
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR pCmdLine, _In_ int nCmdShow)
{
    //int screen_width = GetSystemMetrics(SM_CXSCREEN);
    //int screen_height = GetSystemMetrics(SM_CYSCREEN);

    //char directory[MAX_PATH];
    //GetCurrentDirectory(MAX_PATH, directory);

    gui_initialize();

    GuiWindow* window = gui_create_window(NULL, "Open Battlecry Editor", 0, 0);

    GuiMenu* menu = gui_menu_create();
    GuiMenu* file_menu = gui_submenu_create(menu, "File");
    GuiMenu* tools_menu = gui_submenu_create(menu, "Tools");
    GuiMenu* help_menu = gui_submenu_create(menu, "Help");

    GuiControl* file_menu_quit = gui_menu_add_item(file_menu, "Quit", NULL, quit_callback);
    GuiControl* tools_menu_settings = gui_menu_add_item(tools_menu, "Settings", NULL, NULL);
    GuiControl* help_menu_about = gui_menu_add_item(help_menu, "About", NULL, NULL);

    gui_menu_link(menu, window);

    GuiTab* tab = gui_tab_create(window);
    gui_tab_add_item(tab, "Text");
    gui_tab_add_item(tab, "GUI");
    gui_tab_add_item(tab, "Unit");
    gui_tab_add_item(tab, "Building");
    gui_tab_add_item(tab, "Hero");
    gui_tab_add_item(tab, "Item");

    gui_label_create(window, "BITCHES BE BITCHES!!");

    return gui_message_loop();
}
