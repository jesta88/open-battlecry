#include "system.h"
#include "debug.h"
#include "log.h"
#include "../third_party/tinyfiledialogs/tinyfiledialogs.h"
#include <SDL.h>
#include <stdarg.h>

enum
{
    SYS_MAX_MONITORS = 8,
    SYS_MAX_DISPLAY_MODES = 256
};

static char* base_path;
static char* user_path;
static uint64_t timer_start;
static uint64_t timer_frequency;
static uint32_t monitor_count;
static sys_monitor monitors[SYS_MAX_MONITORS];

//-----------------------------------------------------------------------------
// INITIALIZATION
//-----------------------------------------------------------------------------
void sys_init(const char* org_name, const char* app_name, sys_allocator* allocator)
{
    (void)allocator;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    timer_frequency = SDL_GetPerformanceFrequency();
    timer_start = SDL_GetPerformanceCounter();

    base_path = SDL_GetBasePath();
    user_path = SDL_GetPrefPath(org_name, app_name);

    int display_count = SDL_GetNumVideoDisplays();
    monitor_count = (uint32_t)display_count;
    dbg_assert(monitor_count < SYS_MAX_MONITORS);

    for (int i = 0; i < display_count; i++)
    {
        monitors[i].name = SDL_GetDisplayName(i);
        int display_mode_count = SDL_GetNumDisplayModes(i);
        monitors[i].video_mode_count = display_mode_count;
        dbg_assert(display_mode_count < SYS_MAX_DISPLAY_MODES);

        for (int j = 0; j < display_mode_count; j++)
        {
            SDL_DisplayMode display_mode;
            SDL_GetDisplayMode(i, j, &display_mode);
            monitors[i].video_modes[j] = (sys_video_mode){
                .width = display_mode.w,
                .height = display_mode.h,
                .refresh_rate = display_mode.refresh_rate};
        }
        SDL_DisplayMode display_mode;
        SDL_GetDesktopDisplayMode(i, &display_mode);
        monitors[i].desktop_video_mode = (sys_video_mode){
            .width = display_mode.w,
            .height = display_mode.h,
            .refresh_rate = display_mode.refresh_rate};
    }
}

void sys_quit(void)
{
    SDL_free(user_path);
    SDL_free(base_path);

    SDL_Quit();
}

//-----------------------------------------------------------------------------
// ARGUMENTS
//-----------------------------------------------------------------------------
const char** sys_parse_command_line(int* count)
{
    *count = 0;
    return NULL;
}

bool sys_has_argument(const char* argument)
{
    return false;
}

//-----------------------------------------------------------------------------
// FILE SYSTEM
//-----------------------------------------------------------------------------
uint32_t sys_file_read(const char* path, uint8_t* buffer)
{
}

const char* sys_file_dialog(const char* title, const char* default_path)
{
}

//-----------------------------------------------------------------------------
// TIMER
//-----------------------------------------------------------------------------
static int64_t int64_muldiv(int64_t value, int64_t numer, int64_t denom)
{
    int64_t q = value / denom;
    int64_t r = value % denom;
    return q * numer + r * numer / denom;
}

uint64_t sys_get_ticks(void)
{
    uint64_t ticks = SDL_GetPerformanceCounter();
    return (uint64_t)int64_muldiv(ticks - timer_start, 1000000000, timer_frequency);
}

double sys_get_seconds(uint64_t ticks)
{
    return (double)ticks / 1000000000.0;
}

double sys_get_milliseconds(uint64_t ticks)
{
    return (double)ticks / 1000000.0;
}

double sys_get_microseconds(uint64_t ticks)
{
    return (double)ticks / 1000.0;
}

double sys_get_nanoseconds(uint64_t ticks)
{
    return (double)ticks;
}

//-----------------------------------------------------------------------------
// MEMORY
//-----------------------------------------------------------------------------
void* sys_virtual_allocate(uint64_t size)
{
    (void)size;
    log_error("sys_virtual_allocate is not supported on SDL.");
    return NULL;
}
void sys_virtual_free(void* memory)
{
    (void)memory;
    log_error("sys_virtual_free is not supported on SDL.");
}

void sys_map(const char* path, sys_memory_map* map)
{
    (void)path;
    (void)map;
    log_error("sys_map is not supported on SDL.");
}

void sys_unmap(sys_memory_map* map)
{
    (void)map;
    log_error("sys_unmap is not supported on SDL.");
}

//-----------------------------------------------------------------------------
// THREADING
//-----------------------------------------------------------------------------
void sys_sleep(uint32_t milliseconds)
{
    SDL_Delay(milliseconds);
}

uint32_t sys_get_processor_count(void)
{
    return (uint32_t)SDL_GetCPUCount();
}

sys_thread* sys_create_thread(sys_thread_function thread_function, const char* name, void* user_data)
{
    return SDL_CreateThread(thread_function, name, user_data);
}

void sys_wait_thread(sys_thread* thread, int* return_code)
{
    SDL_WaitThread(thread, return_code);
}

void sys_detatch_thread(sys_thread* thread)
{
    SDL_DetachThread(thread);
}

sys_mutex* sys_create_mutex(void)
{
    return SDL_CreateMutex();
}

void sys_destroy_mutex(sys_mutex* mutex)
{
    SDL_DestroyMutex(mutex);
}

void sys_lock_mutex(sys_mutex* mutex)
{
    SDL_LockMutex(mutex);
}

void sys_unlock_mutex(sys_mutex* mutex)
{
    SDL_UnlockMutex(mutex);
}

sys_condition* sys_create_condition(void)
{
    return SDL_CreateCond();
}

void sys_wait_condition(sys_condition* condition, sys_mutex* mutex)
{
    SDL_CondWait(condition, mutex);
}

void sys_condition_wake_single(sys_condition* condition)
{
    SDL_CondSignal(condition);
}

void sys_condition_wake_all(sys_condition* condition)
{
    SDL_CondBroadcast(condition);
}

//-----------------------------------------------------------------------------
// MONITOR
//-----------------------------------------------------------------------------
sys_monitor* sys_get_monitors(uint32_t* count)
{
    *count = monitor_count;
    return monitors;
}

sys_monitor* sys_get_primary_monitor(void)
{

}

void sys_get_monitor_position(sys_monitor* monitor, int32_t* x, int32_t* y)
{
}

void sys_get_monitor_dpi_scale(sys_monitor* monitor, float* x_scale, float* y_scale)
{
}

const char* sys_get_monitor_name(sys_monitor* monitor)
{
}

const sys_video_mode* sys_get_video_modes(sys_monitor* monitor, uint32_t* count)
{
}

const sys_video_mode* sys_get_video_mode(sys_monitor* monitor)
{
}

//-----------------------------------------------------------------------------
// WINDOW
//-----------------------------------------------------------------------------
sys_window* sys_create_window(const char* title, uint32_t width, uint32_t height, sys_monitor* monitor, uint32_t flags)
{
}
void sys_destroy_window(sys_window* window)
{
}
bool sys_window_should_close(sys_window* window)
{
}
void sys_set_window_title(sys_window* window, const char* title)
{
}
void sys_set_window_icons(sys_window* window, uint32_t count, const c_image* icons)
{
}
void sys_get_window_position(sys_window* window, int32_t* x, int32_t* y)
{
}
void sys_set_window_position(sys_window* window, int32_t x, int32_t y)
{
}
void sys_get_window_size(sys_window* window, uint32_t* width, uint32_t* height)
{
}
void sys_set_window_size(sys_window* window, uint32_t width, uint32_t height)
{
}
void sys_minimize_window(sys_window* window)
{
}
void sys_maximize_window(sys_window* window)
{
}
void sys_restore_window(sys_window* window)
{
}
void sys_show_window(sys_window* window)
{
}
void sys_hide_window(sys_window* window)
{
}
void sys_focus_window(sys_window* window)
{
}
void sys_set_window_monitor(sys_window* window, const sys_monitor* monitor)
{
}

void sys_set_window_size_callback(sys_window* window, sys_window_resize_callback callback)
{
}
void sys_set_window_close_callback(sys_window* window, sys_window_close_callback callback)
{
}
void sys_set_window_focus_callback(sys_window* window, sys_window_focus_callback callback)
{
}
void sys_set_window_minimize_callback(sys_window* window, sys_window_minimize_callback callback)
{
}
void sys_set_window_maximize_callback(sys_window* window, sys_window_maximize_callback callback)
{
}

//-----------------------------------------------------------------------------
// EVENTS
//-----------------------------------------------------------------------------

bool sys_mouse_button_down(sys_window* window, uint32_t button)
{
}

void sys_set_mouse_button_callback(sys_window* window, sys_mouse_button_callback callback)
{
}
void sys_set_mouse_scroll_callback(sys_window* window, sys_mouse_scroll_callback callback)
{
}
void sys_set_mouse_double_click_callback(sys_window* window, sys_mouse_double_click_callback callback)
{

}
void sys_set_mouse_drag_start_callback(sys_window* window, sys_mouse_drag_start_callback callback)
{
}
void sys_set_mouse_drag_callback(sys_window* window, sys_mouse_drag_callback callback)
{
}
void sys_set_mouse_drag_end_callback(sys_window* window, sys_mouse_drag_end_callback callback)
{
}
void sys_set_mouse_key_callback(sys_window* window, sys_key_callback callback)
{
}
void sys_set_mouse_char_callback(sys_window* window, sys_char_callback callback)
{
}

void sys_get_cursor_position(sys_window* window, double* x, double* y)
{
}
void sys_set_cursor_position(sys_window* window, double x, double y)
{
}
void sys_create_cursor(const c_image* icon, int32_t anchor_x, int32_t anchor_y)
{
}