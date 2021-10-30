#include "input.h"
#include "client.h"
#include "../engine/config.h"
#include "../engine/bits.inl"
#include <SDL2/SDL_events.h>
#include <string.h>

enum
{
    C_MAX_KEYBOARD_EVENTS = 64,
    C_MAX_MOUSE_EVENTS = 256,
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

static bool mouse_double_click;
static uint32_t last_click_tick;

static char unicode_chars[C_KEY_PRINTABLE_COUNT];

static const uint32_t doublePressMs = 500;

static const uint8_t scancode_to_key[] = {
    0, 0, 0, 0, C_KEY_A, C_KEY_B, C_KEY_C, C_KEY_D, C_KEY_E, C_KEY_F, C_KEY_G, C_KEY_H, C_KEY_I, C_KEY_J, C_KEY_K, C_KEY_L, C_KEY_M,
    C_KEY_N, C_KEY_O, C_KEY_P, C_KEY_Q, C_KEY_R, C_KEY_S, C_KEY_T, C_KEY_U, C_KEY_V, C_KEY_W, C_KEY_X, C_KEY_Y, C_KEY_Z, C_KEY_1, C_KEY_2,
    C_KEY_3, C_KEY_4, C_KEY_5, C_KEY_6, C_KEY_7, C_KEY_8, C_KEY_9, C_KEY_0, C_KEY_ENTER, C_KEY_ESCAPE, C_KEY_BACKSPACE, C_KEY_TAB,
    C_KEY_SPACE, C_KEY_MINUS, C_KEY_EQUAL, C_KEY_LEFT_BRACKET, C_KEY_RIGHT_BRACKET, C_KEY_BACKSLASH, 0, C_KEY_SEMICOLON,
    C_KEY_APOSTROPHE, C_KEY_GRAVE_ACCENT, C_KEY_COMMA, C_KEY_PERIOD, C_KEY_SLASH, C_KEY_CAPS_LOCK, C_KEY_F1, C_KEY_F2, C_KEY_F3,
    C_KEY_F4, C_KEY_F5, C_KEY_F6, C_KEY_F7, C_KEY_F8, C_KEY_F9, C_KEY_F10, C_KEY_F11, C_KEY_F12, C_KEY_PRINT_SCREEN, C_KEY_SCROLL_LOCK, C_KEY_PAUSE,
    C_KEY_INSERT, C_KEY_HOME, C_KEY_PAGE_UP, C_KEY_DELETE, C_KEY_END, C_KEY_PAGE_DOWN, C_KEY_RIGHT, C_KEY_LEFT, C_KEY_DOWN, C_KEY_UP,
    C_KEY_NUM_LOCK, C_KEY_KP_DIVIDE, C_KEY_KP_MULTIPLY, C_KEY_KP_SUBTRACT, C_KEY_KP_ADD, C_KEY_KP_ENTER, C_KEY_KP_1, C_KEY_KP_2, C_KEY_KP_3,
    C_KEY_KP_4, C_KEY_KP_5, C_KEY_KP_6, C_KEY_KP_7, C_KEY_KP_8, C_KEY_KP_9, C_KEY_KP_0, C_KEY_KP_DECIMAL, 0, 0, 0, C_KEY_KP_EQUAL
};

static void handle_mouse_events(void);
static void handle_keyboard_events(void);

void c_handle_input_events(void)
{
    handle_keyboard_events();
    handle_mouse_events();
}

bool c_mouse_down(enum c_mouse_button mouse_button)
{
    return bits8_is_set(mouse_bitset, mouse_button);
}

bool c_mouse_up(enum c_mouse_button mouse_button)
{
    return !bits8_is_set(mouse_bitset, mouse_button);
}

bool c_mouse_pressed(enum c_mouse_button mouse_button)
{
    return bits8_is_set(mouse_bitset, mouse_button) &&
           !bits8_is_set(previous_mouse_bitset, mouse_button);
}

bool c_mouse_released(enum c_mouse_button mouse_button)
{
    return !bits8_is_set(mouse_bitset, mouse_button) &&
           bits8_is_set(previous_mouse_bitset, mouse_button);
}

bool c_mouse_double_pressed(void)
{
    return 0;
}

bool c_key_down(enum c_key key)
{
    return bits256_is_set(&keys_bitset, key);
}

bool c_key_up(enum c_key key)
{
    return !bits256_is_set(&keys_bitset, key);
}

bool c_key_pressed(enum c_key key)
{
    return bits256_is_set(&keys_bitset, key) && !bits256_is_set(&previous_keys_bitset, key);
}

bool c_key_released(enum c_key key)
{
    return !bits256_is_set(&keys_bitset, key) && bits256_is_set(&previous_keys_bitset, key);
}

static void handle_keyboard_events(void)
{
    memcpy(&previous_keys_bitset, &keys_bitset, sizeof(keys_bitset));

    key_mods = SDL_GetModState();

    SDL_Event events[C_MAX_KEYBOARD_EVENTS];
    const int count = SDL_PeepEvents(
        events, C_MAX_KEYBOARD_EVENTS,
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
                if (c_key_pressed(C_KEY_F4) && is_flag_set(key_mods, KMOD_LALT))
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

    SDL_Event events[C_MAX_MOUSE_EVENTS];
    const int count = SDL_PeepEvents(
        events, C_MAX_MOUSE_EVENTS,
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
