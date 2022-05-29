#pragma once

#include "types.h"

typedef enum 
{
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_4,
    MOUSE_BUTTON_5,
    MOUSE_BUTTON_COUNT
} wb_mouse_button;

typedef enum
{
    KEY_INVALID = 0,
    KEY_SPACE = 32,
    KEY_APOSTROPHE = 39,
    KEY_COMMA = 44,
    KEY_MINUS = 45, 
    KEY_PERIOD = 46,
    KEY_SLASH = 47,
    KEY_0 = 48,
    KEY_1 = 49,
    KEY_2 = 50,
    KEY_3 = 51,
    KEY_4 = 52,
    KEY_5 = 53,
    KEY_6 = 54,
    KEY_7 = 55,
    KEY_8 = 56,
    KEY_9 = 57,
    KEY_SEMICOLON = 59,
    KEY_EQUAL = 61,
    KEY_A = 65,
    KEY_B = 66,
    KEY_C = 67,
    KEY_D = 68,
    KEY_E = 69,
    KEY_F = 70,
    KEY_G = 71,
    KEY_H = 72,
    KEY_I = 73,
    KEY_J = 74,
    KEY_K = 75,
    KEY_L = 76,
    KEY_M = 77,
    KEY_N = 78,
    KEY_O = 79,
    KEY_P = 80,
    KEY_Q = 81,
    KEY_R = 82,
    KEY_S = 83,
    KEY_T = 84,
    KEY_U = 85,
    KEY_V = 86,
    KEY_W = 87,
    KEY_X = 88,
    KEY_Y = 89,
    KEY_Z = 90,
    KEY_LEFT_BRACKET = 91,
    KEY_BACKSLASH = 92,
    KEY_RIGHT_BRACKET = 93,
    KEY_GRAVE_ACCENT = 96,
    KEY_WORLD_1 = 161,
    KEY_WORLD_2 = 162,

    KEY_PRINTABLE_COUNT,
    // End of printable keys

    KEY_ESCAPE,
    KEY_ENTER,
    KEY_TAB,
    KEY_BACKSPACE,
    KEY_INSERT,
    KEY_DELETE,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_HOME,
    KEY_END,
    KEY_CAPS_LOCK,
    KEY_SCROLL_LOCK,
    KEY_NUM_LOCK,
    KEY_PRINT_SCREEN,
    KEY_PAUSE,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,
    KEY_F21,
    KEY_F22,
    KEY_F23,
    KEY_F24,
    KEY_F25,
    KEY_KP_0,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_DECIMAL,
    KEY_KP_DIVIDE,
    KEY_KP_MULTIPLY,
    KEY_KP_SUBTRACT,
    KEY_KP_ADD,
    KEY_KP_ENTER,
    KEY_KP_EQUAL,
    KEY_LEFT_SHIFT,
    KEY_LEFT_CONTROL,
    KEY_LEFT_ALT,
    KEY_LEFT_SUPER,
    KEY_RIGHT_SHIFT,
    KEY_RIGHT_CONTROL,
    KEY_RIGHT_ALT,
    KEY_RIGHT_SUPER,
    KEY_MENU,

    KEY_COUNT
} wb_key;

typedef enum
{
    KEY_MOD_NONE = 0x0000,
    KEY_MOD_LSHIFT = 0x0001,
    KEY_MOD_RSHIFT = 0x0002,
    KEY_MOD_LCTRL = 0x0040,
    KEY_MOD_RCTRL = 0x0080,
    KEY_MOD_LALT = 0x0100,
    KEY_MOD_RALT = 0x0200,
    KEY_MOD_LGUI = 0x0400,
    KEY_MOD_RGUI = 0x0800,
    KEY_MOD_NUM = 0x1000,
    KEY_MOD_CAPS = 0x2000,
    KEY_MOD_MODE = 0x4000,
    KEY_MOD_RESERVED = 0x8000,

    KEY_MOD_CTRL = KEY_MOD_LCTRL | KEY_MOD_RCTRL,
    KEY_MOD_SHIFT = KEY_MOD_LSHIFT | KEY_MOD_RSHIFT,
    KEY_MOD_ALT = KEY_MOD_LALT | KEY_MOD_RALT,
    KEY_MOD_GUI = KEY_MOD_LGUI | KEY_MOD_RGUI
} wb_key_mod;

void wb_input_update(uint16_t key_mod_state);
bool wb_input_handle_key(uint8_t key, bool is_up);
void wb_input_handle_mouse_button(uint8_t mouse_button, bool is_up);
void wb_input_handle_mouse_motion(int32_t x, int32_t y);
void wb_input_handle_mouse_wheel(int32_t x, int32_t y);

bool wb_mouse_down(wb_mouse_button mouse_button);
bool wb_mouse_up(wb_mouse_button mouse_button);
bool wb_mouse_pressed(wb_mouse_button mouse_button);
bool wb_mouse_released(wb_mouse_button mouse_button);
bool wb_mouse_double_pressed(void);

bool wb_key_down(wb_key key);
bool wb_key_up(wb_key key);
bool wb_key_pressed(wb_key key);
bool wb_key_released(wb_key key);