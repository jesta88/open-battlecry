#pragma once

#include "../engine/types.h"

enum c_mouse_button
{
    C_MOUSE_BUTTON_LEFT,
    C_MOUSE_BUTTON_MIDDLE,
    C_MOUSE_BUTTON_RIGHT,
    C_MOUSE_BUTTON_4,
    C_MOUSE_BUTTON_5,
    C_MOUSE_BUTTON_COUNT
};

enum c_key
{
    C_KEY_INVALID = 0,
    C_KEY_SPACE = 32,
    C_KEY_APOSTROPHE = 39,
    C_KEY_COMMA = 44,
    C_KEY_MINUS = 45, 
    C_KEY_PERIOD = 46,
    C_KEY_SLASH = 47,
    C_KEY_0 = 48,
    C_KEY_1 = 49,
    C_KEY_2 = 50,
    C_KEY_3 = 51,
    C_KEY_4 = 52,
    C_KEY_5 = 53,
    C_KEY_6 = 54,
    C_KEY_7 = 55,
    C_KEY_8 = 56,
    C_KEY_9 = 57,
    C_KEY_SEMICOLON = 59,
    C_KEY_EQUAL = 61,
    C_KEY_A = 65,
    C_KEY_B = 66,
    C_KEY_C = 67,
    C_KEY_D = 68,
    C_KEY_E = 69,
    C_KEY_F = 70,
    C_KEY_G = 71,
    C_KEY_H = 72,
    C_KEY_I = 73,
    C_KEY_J = 74,
    C_KEY_K = 75,
    C_KEY_L = 76,
    C_KEY_M = 77,
    C_KEY_N = 78,
    C_KEY_O = 79,
    C_KEY_P = 80,
    C_KEY_Q = 81,
    C_KEY_R = 82,
    C_KEY_S = 83,
    C_KEY_T = 84,
    C_KEY_U = 85,
    C_KEY_V = 86,
    C_KEY_W = 87,
    C_KEY_X = 88,
    C_KEY_Y = 89,
    C_KEY_Z = 90,
    C_KEY_LEFT_BRACKET = 91,
    C_KEY_BACKSLASH = 92,
    C_KEY_RIGHT_BRACKET = 93,
    C_KEY_GRAVE_ACCENT = 96,
    C_KEY_WORLD_1 = 161,
    C_KEY_WORLD_2 = 162,

    C_KEY_PRINTABLE_COUNT,
    // End of printable keys

    C_KEY_ESCAPE,
    C_KEY_ENTER,
    C_KEY_TAB,
    C_KEY_BACKSPACE,
    C_KEY_INSERT,
    C_KEY_DELETE,
    C_KEY_RIGHT,
    C_KEY_LEFT,
    C_KEY_DOWN,
    C_KEY_UP,
    C_KEY_PAGE_UP,
    C_KEY_PAGE_DOWN,
    C_KEY_HOME,
    C_KEY_END,
    C_KEY_CAPS_LOCK,
    C_KEY_SCROLL_LOCK,
    C_KEY_NUM_LOCK,
    C_KEY_PRINT_SCREEN,
    C_KEY_PAUSE,
    C_KEY_F1,
    C_KEY_F2,
    C_KEY_F3,
    C_KEY_F4,
    C_KEY_F5,
    C_KEY_F6,
    C_KEY_F7,
    C_KEY_F8,
    C_KEY_F9,
    C_KEY_F10,
    C_KEY_F11,
    C_KEY_F12,
    C_KEY_F13,
    C_KEY_F14,
    C_KEY_F15,
    C_KEY_F16,
    C_KEY_F17,
    C_KEY_F18,
    C_KEY_F19,
    C_KEY_F20,
    C_KEY_F21,
    C_KEY_F22,
    C_KEY_F23,
    C_KEY_F24,
    C_KEY_F25,
    C_KEY_KP_0,
    C_KEY_KP_1,
    C_KEY_KP_2,
    C_KEY_KP_3,
    C_KEY_KP_4,
    C_KEY_KP_5,
    C_KEY_KP_6,
    C_KEY_KP_7,
    C_KEY_KP_8,
    C_KEY_KP_9,
    C_KEY_KP_DECIMAL,
    C_KEY_KP_DIVIDE,
    C_KEY_KP_MULTIPLY,
    C_KEY_KP_SUBTRACT,
    C_KEY_KP_ADD,
    C_KEY_KP_ENTER,
    C_KEY_KP_EQUAL,
    C_KEY_LEFT_SHIFT,
    C_KEY_LEFT_CONTROL,
    C_KEY_LEFT_ALT,
    C_KEY_LEFT_SUPER,
    C_KEY_RIGHT_SHIFT,
    C_KEY_RIGHT_CONTROL,
    C_KEY_RIGHT_ALT,
    C_KEY_RIGHT_SUPER,
    C_KEY_MENU,

    C_KEY_COUNT
};

enum c_key_mod
{
    C_KEY_MOD_NONE = 0x0000,
    C_KEY_MOD_LSHIFT = 0x0001,
    C_KEY_MOD_RSHIFT = 0x0002,
    C_KEY_MOD_LCTRL = 0x0040,
    C_KEY_MOD_RCTRL = 0x0080,
    C_KEY_MOD_LALT = 0x0100,
    C_KEY_MOD_RALT = 0x0200,
    C_KEY_MOD_LGUI = 0x0400,
    C_KEY_MOD_RGUI = 0x0800,
    C_KEY_MOD_NUM = 0x1000,
    C_KEY_MOD_CAPS = 0x2000,
    C_KEY_MOD_MODE = 0x4000,
    C_KEY_MOD_RESERVED = 0x8000,

    C_KEY_MOD_CTRL = C_KEY_MOD_LCTRL | C_KEY_MOD_RCTRL,
    C_KEY_MOD_SHIFT = C_KEY_MOD_LSHIFT | C_KEY_MOD_RSHIFT,
    C_KEY_MOD_ALT = C_KEY_MOD_LALT | C_KEY_MOD_RALT,
    C_KEY_MOD_GUI = C_KEY_MOD_LGUI | C_KEY_MOD_RGUI
};

void c_handle_input_events(void);

bool c_mouse_down(enum c_mouse_button mouse_button);
bool c_mouse_up(enum c_mouse_button mouse_button);
bool c_mouse_pressed(enum c_mouse_button mouse_button);
bool c_mouse_released(enum c_mouse_button mouse_button);
bool c_mouse_double_pressed(void);

bool c_key_down(enum c_key key);
bool c_key_up(enum c_key key);
bool c_key_pressed(enum c_key key);
bool c_key_released(enum c_key key);
