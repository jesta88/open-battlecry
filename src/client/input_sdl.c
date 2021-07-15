#include "input.h"
#include "client.h"
#include "../shared/config.h"
#include "../shared/bits.inl"
#include <SDL2/SDL_events.h>

enum
{
    C_MAX_KEYBOARD_EVENTS = 64,
    C_MAX_MOUSE_EVENTS = 256,
};

struct WbInput
{
    WbBitset256 keys_bitset;
    WbBitset256 previous_keys_bitset;
    WbBitset8 mouse_bitset;
    WbBitset8 previous_mouse_bitset;

    int32_t mouse_delta_x;
    int32_t mouse_delta_y;
    int32_t mouse_wheel_x;
    int32_t mouse_wheel_y;

    SDL_Keymod key_mods;

    bool mouse_double_click;
    uint32_t last_click_tick;

    char unicode_chars[C_KEY_PRINTABLE_COUNT];
};

static WbInput input;

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

static void handleMouseEvents(void);
static void handleKeyboardEvents(void);

void wbHandleInputEvents(void)
{
    handleKeyboardEvents();
    handleMouseEvents();
}

bool wbMouseDown(enum c_mouse_button mouse_button)
{
    return wbIsBitSet8(input.mouse_bitset, mouse_button);
}

bool wbMouseUp(enum c_mouse_button mouse_button)
{
    return !wbIsBitSet8(input.mouse_bitset, mouse_button);
}

bool wbMousePressed(enum c_mouse_button mouse_button)
{
    return wbIsBitSet8(input.mouse_bitset, mouse_button) &&
        !wbIsBitSet8(input.previous_mouse_bitset, mouse_button);
}

bool wbMouseReleased(enum c_mouse_button mouse_button)
{
    return !wbIsBitSet8(input.mouse_bitset, mouse_button) &&
        wbIsBitSet8(input.previous_mouse_bitset, mouse_button);
}

bool wbMouseDoublePressed(void)
{
    return 0;
}

bool wbKeyDown(enum c_key key)
{
    return wbIsBitSet256(&input.keys_bitset, key);
}

bool wbKeyUp(enum c_key key)
{
    return !wbIsBitSet256(&input.keys_bitset, key);
}

bool wbKeyPressed(enum c_key key)
{
    return wbIsBitSet256(&input.keys_bitset, key) && !wbIsBitSet256(&input.previous_keys_bitset, key);
}

bool wbKeyReleased(enum c_key key)
{
    return !wbIsBitSet256(&input.keys_bitset, key) && wbIsBitSet256(&input.previous_keys_bitset, key);
}

static void handleKeyboardEvents(void)
{
    memcpy(&input.previous_keys_bitset, &input.keys_bitset, sizeof(input.keys_bitset));

    input.key_mods = SDL_GetModState();

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
            wbSetBit256(&input.keys_bitset, key);

#ifdef _WIN32
            if (wbKeyPressed(C_KEY_F4) && wbHasFlag32(input.key_mods, KMOD_LALT))
            {
                c_quit->bool_value = true;
            }
#endif
            break;
        }

        case SDL_KEYUP:
        {
            uint8_t key = scancode_to_key[event.key.keysym.scancode];
            wbClearBit256(&input.keys_bitset, key);
            break;
        }

        default:
            break;
        }
    }
}

static void handleMouseEvents(void)
{
    input.previous_mouse_bitset = input.mouse_bitset;

    SDL_Event events[C_MAX_MOUSE_EVENTS];
    const int count = SDL_PeepEvents(
        events, C_MAX_MOUSE_EVENTS,
        SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEWHEEL);

    input.mouse_delta_x = input.mouse_delta_y = input.mouse_wheel_x = input.mouse_wheel_y = 0;

    for (int i = 0; i < count; ++i)
    {
        const SDL_Event event = events[i];

        switch (event.type)
        {
        case SDL_MOUSEMOTION:
            //                mouseTimeStamp[0] = m_curTime + event.motion.timestamp;
            //                mouseTimeStamp[1] = m_curTime + event.motion.timestamp;
            input.mouse_delta_x += event.motion.xrel;
            input.mouse_delta_y += event.motion.yrel;
            break;

        case SDL_MOUSEBUTTONDOWN:
            wbSetBit8(&input.mouse_bitset, event.button.button - 1);
            // TODO: Callback
            break;

        case SDL_MOUSEBUTTONUP:
            wbClearBit8(&input.mouse_bitset, event.button.button - 1);
            // TODO: Callback
            break;

        case SDL_MOUSEWHEEL:
            //                mouseTimeStamp[2] = m_curTime + event.wheel.timestamp;
            //                mouseTimeStamp[3] = m_curTime + event.wheel.timestamp;
            input.mouse_wheel_y += event.wheel.y;
            input.mouse_wheel_x += event.wheel.x;
            break;
        }
    }
}
