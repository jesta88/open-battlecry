#ifndef SERVER
#include "private.h"
#include "core/bits.inl"
#include <engine/input.h>
#include <engine/config.h>
#include <SDL2/SDL_events.h>
#include <string.h>

enum
{
    MAX_KEYBOARD_EVENTS = 64,
    MAX_MOUSE_EVENTS = 256,
};

static struct bits256_t keys_bitset;
static struct bits256_t previous_keys_bitset;
static struct bits8_t mouse_bitset;
static struct bits8_t previous_mouse_bitset;

static int32_t mouse_delta_x;
static int32_t mouse_delta_y;
static int32_t mouse_wheel_x;
static int32_t mouse_wheel_y;

static SDL_Keymod key_mods;

//static bool mouse_double_click;
//static uint32_t last_click_tick;

//static char unicode_chars[KEY_PRINTABLE_COUNT];

static const uint8_t scancode_to_key[] = {
    0, 0, 0, 0, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
    KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_1, KEY_2,
    KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_ENTER, KEY_ESCAPE, KEY_BACKSPACE, KEY_TAB,
    KEY_SPACE, KEY_MINUS, KEY_EQUAL, KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET, KEY_BACKSLASH, 0, KEY_SEMICOLON,
    KEY_APOSTROPHE, KEY_GRAVE_ACCENT, KEY_COMMA, KEY_PERIOD, KEY_SLASH, KEY_CAPS_LOCK, KEY_F1, KEY_F2, KEY_F3,
    KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_PRINT_SCREEN, KEY_SCROLL_LOCK, KEY_PAUSE,
    KEY_INSERT, KEY_HOME, KEY_PAGE_UP, KEY_DELETE, KEY_END, KEY_PAGE_DOWN, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP,
    KEY_NUM_LOCK, KEY_KP_DIVIDE, KEY_KP_MULTIPLY, KEY_KP_SUBTRACT, KEY_KP_ADD, KEY_KP_ENTER, KEY_KP_1, KEY_KP_2, KEY_KP_3,
    KEY_KP_4, KEY_KP_5, KEY_KP_6, KEY_KP_7, KEY_KP_8, KEY_KP_9, KEY_KP_0, KEY_KP_DECIMAL, 0, 0, 0, KEY_KP_EQUAL
};

static void handle_mouse_events(void);
static void handle_keyboard_events(void);

void handle_input_events(void)
{
    handle_keyboard_events();
    handle_mouse_events();
}

bool mouse_down(enum mouse_button mouse_button)
{
    return bits8_is_set(mouse_bitset, mouse_button);
}

bool mouse_up(enum mouse_button mouse_button)
{
    return !bits8_is_set(mouse_bitset, mouse_button);
}

bool mouse_pressed(enum mouse_button mouse_button)
{
    return bits8_is_set(mouse_bitset, mouse_button) &&
           !bits8_is_set(previous_mouse_bitset, mouse_button);
}

bool mouse_released(enum mouse_button mouse_button)
{
    return !bits8_is_set(mouse_bitset, mouse_button) &&
           bits8_is_set(previous_mouse_bitset, mouse_button);
}

bool mouse_double_pressed(void)
{
    return 0;
}

bool key_down(enum key key)
{
    return bits256_is_set(&keys_bitset, key);
}

bool key_up(enum key key)
{
    return !bits256_is_set(&keys_bitset, key);
}

bool key_pressed(enum key key)
{
    return bits256_is_set(&keys_bitset, key) && !bits256_is_set(&previous_keys_bitset, key);
}

bool key_released(enum key key)
{
    return !bits256_is_set(&keys_bitset, key) && bits256_is_set(&previous_keys_bitset, key);
}

static void handle_keyboard_events(void)
{
    memcpy(&previous_keys_bitset, &keys_bitset, sizeof(keys_bitset));

    key_mods = SDL_GetModState();

    SDL_Event events[MAX_KEYBOARD_EVENTS];
    const int count = SDL_PeepEvents(
        events, MAX_KEYBOARD_EVENTS,
        SDL_GETEVENT, SDL_KEYDOWN, SDL_TEXTINPUT);

    for (int i = 0; i < count; i++)
    {
        const SDL_Event event = events[i];
        switch (event.type)
        {
            case SDL_KEYDOWN:
            {
                if (event.key.repeat)
                    continue;

                uint8_t key = scancode_to_key[event.key.keysym.scancode];
                bits256_set(&keys_bitset, key);

#ifdef _WIN32
                // Quit on Alt-F4
                if (key_pressed(KEY_F4) && is_flag_set(key_mods, KMOD_LALT))
                {
                    c_quit->bool_value = true;
                }
#endif
                break;
            }

            case SDL_KEYUP:
            {
                uint8_t key = scancode_to_key[event.key.keysym.scancode];
                bits256_clear(&keys_bitset, key);
                break;
            }

            default:
                break;
        }
    }
}

static void handle_mouse_events(void)
{
    previous_mouse_bitset = mouse_bitset;

    SDL_Event events[MAX_MOUSE_EVENTS];
    const int count = SDL_PeepEvents(
        events, MAX_MOUSE_EVENTS,
        SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEWHEEL);

    mouse_delta_x = mouse_delta_y = mouse_wheel_x = mouse_wheel_y = 0;

    for (int i = 0; i < count; ++i)
    {
        const SDL_Event event = events[i];

        switch (event.type)
        {
            case SDL_MOUSEMOTION:
                //                mouseTimeStamp[0] = m_curTime + event.motion.timestamp;
                //                mouseTimeStamp[1] = m_curTime + event.motion.timestamp;
                mouse_delta_x += event.motion.xrel;
                mouse_delta_y += event.motion.yrel;
                break;

            case SDL_MOUSEBUTTONDOWN:
                bits8_set(&mouse_bitset, event.button.button - 1);
                break;

            case SDL_MOUSEBUTTONUP:
                bits8_clear(&mouse_bitset, event.button.button - 1);
                break;

            case SDL_MOUSEWHEEL:
                //                mouseTimeStamp[2] = m_curTime + event.wheel.timestamp;
                //                mouseTimeStamp[3] = m_curTime + event.wheel.timestamp;
                mouse_wheel_y += event.wheel.y;
                mouse_wheel_x += event.wheel.x;
                break;
        }
    }
}
#endif