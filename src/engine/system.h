#pragma once

#include "std.h"

//-----------------------------------------------------------------------------
// INITIALIZATION
//-----------------------------------------------------------------------------
typedef struct
{
    void* (*allocate)(uint64_t size);
    void (*deallocate)(void* memory);
    void* user_data;
} sys_allocator;

ENGINE_API void sys_init(const char* org_name, const char* app_name, sys_allocator* allocator);
ENGINE_API void sys_quit(void);

//-----------------------------------------------------------------------------
// ARGUMENTS
//-----------------------------------------------------------------------------
ENGINE_API const char** sys_parse_command_line(int* count);
ENGINE_API bool sys_has_argument(const char* argument);

//-----------------------------------------------------------------------------
// FILE SYSTEM
//-----------------------------------------------------------------------------
ENGINE_API uint32_t sys_file_read(const char* path, uint8_t* buffer);
ENGINE_API const char* sys_file_dialog(const char* title, const char* default_path);

//-----------------------------------------------------------------------------
// TIMER
//-----------------------------------------------------------------------------
ENGINE_API uint64_t sys_get_ticks(void);
ENGINE_API double sys_get_seconds(uint64_t ticks);
ENGINE_API double sys_get_milliseconds(uint64_t ticks);
ENGINE_API double sys_get_microseconds(uint64_t ticks);
ENGINE_API double sys_get_nanoseconds(uint64_t ticks);

//-----------------------------------------------------------------------------
// MEMORY
//-----------------------------------------------------------------------------
typedef struct
{
    const void* data;
    void* map_object;
} sys_memory_map;

ENGINE_API void* sys_virtual_allocate(uint64_t size);
ENGINE_API void sys_virtual_free(void* memory);

ENGINE_API void sys_map(const char* path, sys_memory_map* map);
ENGINE_API void sys_unmap(sys_memory_map* map);

//-----------------------------------------------------------------------------
// THREADING
//-----------------------------------------------------------------------------
typedef struct SDL_Thread sys_thread;
typedef struct SDL_mutex sys_mutex;
typedef struct SDL_cond sys_condition;

typedef int (*sys_thread_function)(void* user_data);

ENGINE_API void sys_sleep(uint32_t milliseconds);
ENGINE_API uint32_t sys_get_processor_count(void);

ENGINE_API sys_thread* sys_create_thread(sys_thread_function thread_function, const char* name, void* user_data);
ENGINE_API void sys_wait_thread(sys_thread* thread, int* return_code);
ENGINE_API void sys_detatch_thread(sys_thread* thread);

ENGINE_API sys_mutex* sys_create_mutex(void);
ENGINE_API void sys_destroy_mutex(sys_mutex* mutex);
ENGINE_API void sys_lock_mutex(sys_mutex* mutex);
ENGINE_API void sys_unlock_mutex(sys_mutex* mutex);

ENGINE_API sys_condition* sys_create_condition(void);
ENGINE_API void sys_wait_condition(sys_condition* condition, sys_mutex* mutex);
ENGINE_API void sys_condition_wake_single(sys_condition* condition);
ENGINE_API void sys_condition_wake_all(sys_condition* condition);

//-----------------------------------------------------------------------------
// MONITOR
//-----------------------------------------------------------------------------
typedef struct
{
    uint32_t width;
    uint32_t height;
    uint32_t refresh_rate;
} sys_video_mode;

typedef struct
{
    const char* name;
    uint32_t video_mode_count;
    sys_video_mode video_modes[256];
    sys_video_mode desktop_video_mode;
} sys_monitor;

ENGINE_API sys_monitor* sys_get_monitors(uint32_t* count);
ENGINE_API sys_monitor* sys_get_primary_monitor(void);
ENGINE_API void sys_get_monitor_position(sys_monitor* monitor, int32_t* x, int32_t* y);
ENGINE_API void sys_get_monitor_dpi_scale(sys_monitor* monitor, float* x_scale, float* y_scale);
ENGINE_API const char* sys_get_monitor_name(sys_monitor* monitor);
ENGINE_API const sys_video_mode* sys_get_video_modes(sys_monitor* monitor, uint32_t* count);
ENGINE_API const sys_video_mode* sys_get_video_mode(sys_monitor* monitor);

//-----------------------------------------------------------------------------
// WINDOW
//-----------------------------------------------------------------------------
enum sys_window_flags
{
    SYS_WINDOW_FULLSCREEN = 0x01,
    SYS_WINDOW_BORDERLESS = 0x02,
    SYS_WINDOW_RESIZABLE = 0x04,
    SYS_WINDOW_MAXIMIZED = 0x08
};

typedef struct SDL_Window sys_window;

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint8_t* pixels;
} c_image;

typedef void (*sys_window_resize_callback)(sys_window* window, uint32_t width, uint32_t height);
typedef void (*sys_window_close_callback)(sys_window* window);
typedef void (*sys_window_focus_callback)(sys_window* window, bool focused);
typedef void (*sys_window_minimize_callback)(sys_window* window);
typedef void (*sys_window_maximize_callback)(sys_window* window);

typedef void (*sys_mouse_button_callback)(sys_window* window, uint32_t button, bool pressed, uint32_t mods);
typedef void (*sys_mouse_scroll_callback)(sys_window* window, double x_offset, double y_offset);
typedef void (*sys_mouse_double_click_callback)(sys_window* window);
typedef void (*sys_mouse_drag_start_callback)(sys_window* window, int32_t x, int32_t y);
typedef void (*sys_mouse_drag_callback)(sys_window* window, int32_t x, int32_t y);
typedef void (*sys_mouse_drag_end_callback)(sys_window* window, int32_t x, int32_t y);
typedef void (*sys_key_callback)(sys_window* window, uint32_t key, bool pressed, bool repeated, uint32_t mods);
typedef void (*sys_char_callback)(sys_window* window, uint32_t unicode_codepoint);
typedef void (*sys_char_mod_callback)(sys_window* window, uint32_t unicode_codepoint, uint32_t mods);

ENGINE_API sys_window* sys_create_window(const char* title, uint32_t width, uint32_t height, sys_monitor* monitor, uint32_t flags);
ENGINE_API void sys_destroy_window(sys_window* window);
ENGINE_API bool sys_window_should_close(sys_window* window);
ENGINE_API void sys_set_window_title(sys_window* window, const char* title);
ENGINE_API void sys_set_window_icons(sys_window* window, uint32_t count, const c_image* icons);
ENGINE_API void sys_get_window_position(sys_window* window, int32_t* x, int32_t* y);
ENGINE_API void sys_set_window_position(sys_window* window, int32_t x, int32_t y);
ENGINE_API void sys_get_window_size(sys_window* window, uint32_t* width, uint32_t* height);
ENGINE_API void sys_set_window_size(sys_window* window, uint32_t width, uint32_t height);
ENGINE_API void sys_minimize_window(sys_window* window);
ENGINE_API void sys_maximize_window(sys_window* window);
ENGINE_API void sys_restore_window(sys_window* window);
ENGINE_API void sys_show_window(sys_window* window);
ENGINE_API void sys_hide_window(sys_window* window);
ENGINE_API void sys_focus_window(sys_window* window);
ENGINE_API struct sys_monitor* sys_get_window_monitor(sys_window* window);
ENGINE_API void sys_set_window_monitor(sys_window* window, const sys_monitor* monitor);

ENGINE_API void sys_set_window_size_callback(sys_window* window, sys_window_resize_callback callback);
ENGINE_API void sys_set_window_close_callback(sys_window* window, sys_window_close_callback callback);
ENGINE_API void sys_set_window_focus_callback(sys_window* window, sys_window_focus_callback callback);
ENGINE_API void sys_set_window_minimize_callback(sys_window* window, sys_window_minimize_callback callback);
ENGINE_API void sys_set_window_maximize_callback(sys_window* window, sys_window_maximize_callback callback);

//-----------------------------------------------------------------------------
// EVENTS
//-----------------------------------------------------------------------------
enum sys_event_type
{
    SYS_EVENT_QUIT,
    SYS_EVENT_WINDOW_RESIZE,
    SYS_EVENT_WINDOW_CLOSE,
    SYS_EVENT_KEY,
    SYS_EVENT_TEXT,
    SYS_EVENT_MOUSE_MOVE,
    SYS_EVENT_MOUSE_BUTTON,
    SYS_EVENT_MOUSE_WHEEL,
    SYS_EVENT_CLIPBOARD,
    SYS_EVENT_FORCE_U32 = 0x7FFFFFFF
};

enum sys_mouse_button
{
    SYS_MOUSE_BUTTON_1 = 0,
    SYS_MOUSE_BUTTON_2 = 1,
    SYS_MOUSE_BUTTON_3 = 2,
    SYS_MOUSE_BUTTON_4 = 3,
    SYS_MOUSE_BUTTON_5 = 4,
    SYS_MOUSE_BUTTON_6 = 5,
    SYS_MOUSE_BUTTON_7 = 6,
    SYS_MOUSE_BUTTON_8 = 7,
    SYS_MOUSE_BUTTON_LAST = SYS_MOUSE_BUTTON_8,
    SYS_MOUSE_BUTTON_LEFT = SYS_MOUSE_BUTTON_1,
    SYS_MOUSE_BUTTON_RIGHT = SYS_MOUSE_BUTTON_2,
    SYS_MOUSE_BUTTON_MIDDLE = SYS_MOUSE_BUTTON_3
};

struct sys_event
{
    uint8_t type;
    uint32_t timestamp;
    union {
        struct window_resize
        {
            uint32_t width;
            uint32_t height;
        } window_resize;
        struct key
        {
            uint16_t keycode;
            bool pressed;
            bool repeat;
        } key;
        char text[8];
        struct mouse_move
        {
            int32_t x;
            int32_t y;
        } mouse_move;
        struct mouse_button
        {
            uint8_t button;
            uint8_t clicks;
            bool pressed;
        } mouse_button;
        struct mouse_wheel
        {
            int32_t scroll;
        } mouse_wheel;
    };
};
_Static_assert(sizeof(struct sys_event) == 16, "sizeof(struct sys_event) != 16");

typedef void (*sys_window_resize_callback)(sys_window* window, uint32_t width, uint32_t height);
typedef void (*sys_window_close_callback)(sys_window* window);
typedef void (*sys_window_focus_callback)(sys_window* window, bool focused);
typedef void (*sys_window_minimize_callback)(sys_window* window);
typedef void (*sys_window_maximize_callback)(sys_window* window);

typedef void (*sys_mouse_button_callback)(sys_window* window, uint32_t mouse_button, bool pressed, uint32_t mods);
typedef void (*sys_mouse_scroll_callback)(sys_window* window, double x_offset, double y_offset);
typedef void (*sys_mouse_double_click_callback)(sys_window* window);
typedef void (*sys_mouse_drag_start_callback)(sys_window* window, int32_t x, int32_t y);
typedef void (*sys_mouse_drag_callback)(sys_window* window, int32_t x, int32_t y);
typedef void (*sys_mouse_drag_end_callback)(sys_window* window, int32_t x, int32_t y);
typedef void (*sys_key_callback)(sys_window* window, uint32_t key, bool pressed, bool repeated, uint32_t mods);
typedef void (*sys_char_callback)(sys_window* window, uint32_t unicode_codepoint);
typedef void (*sys_char_mod_callback)(sys_window* window, uint32_t unicode_codepoint, uint32_t mods);

ENGINE_API bool sys_mouse_button_down(sys_window* window, uint32_t button);

ENGINE_API void sys_set_mouse_button_callback(sys_window* window, sys_mouse_button_callback callback);
ENGINE_API void sys_set_mouse_scroll_callback(sys_window* window, sys_mouse_scroll_callback callback);
ENGINE_API void sys_set_mouse_double_click_callback(sys_window* window, sys_mouse_double_click_callback callback);
ENGINE_API void sys_set_mouse_drag_start_callback(sys_window* window, sys_mouse_drag_start_callback callback);
ENGINE_API void sys_set_mouse_drag_callback(sys_window* window, sys_mouse_drag_callback callback);
ENGINE_API void sys_set_mouse_drag_end_callback(sys_window* window, sys_mouse_drag_end_callback callback);
ENGINE_API void sys_set_mouse_key_callback(sys_window* window, sys_key_callback callback);
ENGINE_API void sys_set_mouse_char_callback(sys_window* window, sys_char_callback callback);

ENGINE_API void sys_get_cursor_position(sys_window* window, double* x, double* y);
ENGINE_API void sys_set_cursor_position(sys_window* window, double x, double y);
ENGINE_API void sys_create_cursor(const c_image* icon, int32_t anchor_x, int32_t anchor_y);
