#pragma once

#include "types.h"

typedef struct
{
    void (*init)(void);
    void (*quit)(void);

    void (*reload)(void);
    void (*unload)(void);

    void (*update)(float delta_time);
    void (*draw)(void);

    char org_name[64];
    char app_name[64];
} wb_engine_desc;

typedef struct wb_engine wb_engine;

wb_engine* wb_engine_init(const wb_engine_desc* engine_desc);
void wb_engine_quit(wb_engine* engine);
void wb_message_box(const char* title, const char* message);
      
void wb_toggle_borderless(uint32_t width, uint32_t height);
void wb_toggle_fullscreen(void);
void wb_get_window_size(uint16_t* width, uint16_t* height);
      
void wb_show_cursor(void);
void wb_hide_cursor(void);
bool wb_is_cursor_inside_window(void);
      
void wb_get_default_resolution(uint32_t* width, uint32_t* height);
void wb_set_resolution(uint32_t width, uint32_t height);
void wb_get_dpi(float* x, float* y);

uint32_t wb_get_monitor_count(void);
struct ws_monitor* wb_get_monitor(uint32_t index);

const char* wb_get_root_path(void);
const char* wb_get_user_path(void);
