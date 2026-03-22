#pragma once

#include "platform.h"
#include "types.h"

typedef struct wk_app wk_app;

typedef struct wk_app_state
{
    int32 argc;
    const char** argv;

    uint32 window_width;
    uint32 window_height;
    int32 window_x;
    int32 window_y;

    uint8 monitor_index;
    bool fullscreen;
    bool borderless;
    bool low_dpi;
} wk_app_state;

typedef struct wk_app_desc
{
    void (*init)(void);
    void (*tick)(void);
    void (*quit)(void);

    const char* title;
    const char* org;
} wk_app_desc;

WK_API int wk_main(int argc, char* argv[], wk_app_desc* app_desc);

int wk_create_graphics_app(int argc, char* argv[]);
int wk_create_console_app(int argc, char* argv[]);
int wk_create_gui_app(int argc, char* argv[]);