#pragma once

#include "types.h"

typedef struct ws_app_desc
{
    void (*init)(void);
    void (*quit)(void);

    void (*reload)(void);
    void (*unload)(void);

    void (*update)(float delta_time);
    void (*draw)(void);

    char org_name[64];
    char app_name[64];
} ws_app_desc;

// This needs to be defined by the game.
extern ws_app_desc ws_main(int argc, char* argv[]);

void ws_quit(void);
void ws_message_box(const char* title, const char* message);

void ws_toggle_borderless(uint32_t width, uint32_t height);
void ws_toggle_fullscreen(void);
void ws_get_window_size(uint16_t* width, uint16_t* height);

void ws_show_cursor(void);
void ws_hide_cursor(void);
bool ws_is_cursor_inside_window(void);

void ws_get_default_resolution(uint32_t* width, uint32_t* height);
void ws_set_resolution(uint32_t width, uint32_t height);
void ws_get_dpi(float* x, float* y);

uint32_t ws_get_monitor_count(void);
struct ws_monitor* ws_get_monitor(uint32_t index);

const char* ws_get_root_path(void);
const char* ws_get_user_path(void);
