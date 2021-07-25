#pragma once

#include <stdbool.h>

typedef struct GuiWindow GuiWindow;
typedef struct GuiMenu GuiMenu;
typedef struct GuiTab GuiTab;

void gui_initialize(void);
int gui_message_loop(void);
void gui_quit(void); 

GuiWindow* gui_create_window(GuiWindow* owner, const char* title, int width, int height);

GuiMenu* gui_menu_begin(GuiWindow* window);
void gui_menu_end(GuiWindow* window, GuiMenu* menu);
GuiMenu* gui_menu_add(void);
void gui_menu_add_item(GuiMenu* menu, const char* name);

GuiTab* gui_create_tab(GuiWindow* owner);
void gui_add_tab_item(GuiTab* tab, const char* name);