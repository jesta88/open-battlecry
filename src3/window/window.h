#ifndef WK_HEADLESS
#pragma once

#include "types.h"

enum
{
    WK_DEFAULT_WINDOW_WIDTH = 1280,
    WK_DEFAULT_WINDOW_HEIGHT = 720
};

enum
{
    WK_WINDOW_EVENT_QUIT,
    WK_WINDOW_EVENT_RESIZED,
    WK_WINDOW_EVENT_CLOSED,
    WK_WINDOW_EVENT_KEY,
    WK_WINDOW_EVENT_CHAR,
    WK_WINDOW_EVENT_MOUSE_BUTTON,
    WK_WINDOW_EVENT_MOUSE_MOVE,
    WK_WINDOW_EVENT_MOUSE_WHEEL,

    WK_WINDOW_EVENT_COUNT
};

typedef struct
{
    uint16_t type;
    union
    {
        struct
        {
            uint16_t width;
            uint16_t height;
        } resize;
        struct
        {
            uint16_t keycode;
            uint16_t scancode;
            uint8 modifiers;
            bool pressed;
        } key;
        struct
        {
            uint16_t character;
        } text;
        struct
        {
            uint16_t button;
            uint16_t x;
            uint16_t y;
        } mouse;
    };
} wk_event;

typedef struct
{
    const char* title;
    uint16_t width;
    uint16_t height;
    bool fullscreen;
} wk_window_desc;

int wk_open_window(const wk_window_desc* window_desc);
bool wk_poll_events(wk_event* event);
void wk_show_window(void);
void wk_close_window(void);

#if WK_PLATFORM_WINDOWS
void* wk_get_win32_hwnd(void);
#endif
#endif // WK_HEADLESS