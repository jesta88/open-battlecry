#pragma once

#include "types.h"

struct app_desc
{
    void (*init)(void);
    void (*quit)(void);

    void (*reload)(void);
    void (*unload)(void);

    void (*update)(float delta_time);
    void (*draw)(void);

    char org_name[64];
    char app_name[64];
};

// This needs to be defined by the game.
extern struct app_desc app_main(int argc, char* argv[]);

void app_quit(void);
void app_message_box(const char* title, const char* message);

void app_toggle_borderless(uint32_t width, uint32_t height);
void app_toggle_fullscreen(void);
void app_show(void);
void app_hide(void);
void app_maximize(void);
void app_minimize(void);
void app_get_window_size(uint32_t* width, uint32_t* height);

void app_show_cursor(void);
void app_hide_cursor(void);
bool app_is_cursor_inside_window(void);

void app_get_default_resolution(uint32_t* width, uint32_t* height);
void app_set_resolution(uint32_t width, uint32_t height);
void app_get_dpi(float* x, float* y);

uint32_t app_get_monitor_count(void);
struct monitor* app_get_monitor(uint32_t index);

const char* app_get_root_path(void);
const char* app_get_user_path(void);
