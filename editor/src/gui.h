#pragma once

#include <stdbool.h>

typedef struct GuiWindow GuiWindow;
typedef struct GuiIcon GuiIcon;
typedef struct GuiMenu GuiMenu;
typedef struct GuiControl GuiControl;
typedef struct GuiTab GuiTab;

typedef void (*gui_callback)(void);

enum gui_message
{
    GUI_MSG_PAINT,
    GUI_MSG_LAYOUT,
    GUI_MSG_DESTROY,

    GUI_MSG_WINDOW_CLOSE,

    GUI_MSG_MOUSE_LEFT_DOWN,
    GUI_MSG_MOUSE_LEFT_UP,
    GUI_MSG_MOUSE_MIDDLE_DOWN,
    GUI_MSG_MOUSE_MIDDLE_UP,
    GUI_MSG_MOUSE_RIGHT_DOWN,
    GUI_MSG_MOUSE_RIGHT_UP,
    GUI_MSG_MOUSE_MOVE,
    GUI_MSG_MOUSE_DRAG,
    GUI_MSG_MOUSE_WHEEL,
};

void gui_initialize(void);
int  gui_message_loop(void);
void gui_quit(void);

GuiWindow* gui_create_window(GuiWindow* owner, const char* title, int width, int height);

GuiMenu* gui_menu_create(void);
GuiMenu* gui_submenu_create(GuiMenu* parent, const char* name);
void     gui_menu_link(GuiMenu* menu, GuiWindow* window);
GuiControl* gui_menu_add_item(GuiMenu* menu, const char* name, const GuiIcon* icon, gui_callback callback);

GuiTab* gui_tab_create(GuiWindow* owner);
void    gui_tab_add_item(GuiTab* tab, const char* name);