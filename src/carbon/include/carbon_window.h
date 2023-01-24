#pragma once

#include "../types.h"

typedef enum
{
    C_WINDOW_FULLSCREEN = 0x01,
    C_WINDOW_BORDERLESS = 0x02,
    C_WINDOW_RESIZABLE = 0x04,
    C_WINDOW_MAXIMIZED = 0x08,
    C_WINDOW_MINIMIZED = 0x10
} c_window_flags;

typedef struct c_window c_window;

typedef struct
{
    int width;
    int height;
    int refresh_rate;
} c_display_mode;

typedef struct
{
    const char* name;
    int video_mode_count;
    c_display_mode video_modes[256];
    c_display_mode desktop_video_mode;
} c_monitor;

typedef struct
{
    int width;
    int height;
    uint8_t* pixels;
} c_image;

typedef void (*c_window_resize_callback)(c_window* window, int width, int height);
typedef void (*c_window_close_callback)(c_window* window);
typedef void (*c_window_focus_callback)(c_window* window, bool focused);
typedef void (*c_window_minimize_callback)(c_window* window);
typedef void (*c_window_maximize_callback)(c_window* window);

c_window* c_create_window(const char* title, int width, int height, c_monitor* monitor, c_window_flags flags);
void c_destroy_window(c_window* window);
bool c_window_should_close(c_window* window);
void c_set_window_title(c_window* window, const char* title);
void c_set_window_icons(c_window* window, int count, const c_image* icons);
void c_get_window_position(c_window* window, int* x, int* y);
void c_set_window_position(c_window* window, int x, int y);
void c_get_window_size(c_window* window, int* width, int* height);
void c_set_window_size(c_window* window, int width, int height);
void c_minimize_window(c_window* window);
void c_maximize_window(c_window* window);
void c_restore_window(c_window* window);
void c_show_window(c_window* window);
void c_hide_window(c_window* window);
void c_focus_window(c_window* window);
void c_set_window_monitor(c_window* window, const c_monitor* monitor);
c_monitor* c_get_window_monitor(c_window* window);

void c_set_window_size_callback(c_window* window, c_window_resize_callback callback);
void c_set_window_close_callback(c_window* window, c_window_close_callback callback);
void c_set_window_focus_callback(c_window* window, c_window_focus_callback callback);
void c_set_window_minimize_callback(c_window* window, c_window_minimize_callback callback);
void c_set_window_maximize_callback(c_window* window, c_window_maximize_callback callback);