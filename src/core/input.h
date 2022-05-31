#pragma once

#include "types.h"

typedef enum 
{
    WB_MOUSE_BUTTON_LEFT,
    WB_MOUSE_BUTTON_MIDDLE,
    WB_MOUSE_BUTTON_RIGHT,
    WB_MOUSE_BUTTON_4,
    WB_MOUSE_BUTTON_5,
    WB_MOUSE_BUTTON_COUNT
} wb_mouse_button;

typedef enum
{
    WB_KEY_INVALID = 0,
    WB_KEY_SPACE = 32,
    WB_KEY_APOSTROPHE = 39,
    WB_KEY_COMMA = 44,
    WB_KEY_MINUS = 45, 
    WB_KEY_PERIOD = 46,
    WB_KEY_SLASH = 47,
    WB_KEY_0 = 48,
    WB_KEY_1 = 49,
    WB_KEY_2 = 50,
    WB_KEY_3 = 51,
    WB_KEY_4 = 52,
    WB_KEY_5 = 53,
    WB_KEY_6 = 54,
    WB_KEY_7 = 55,
    WB_KEY_8 = 56,
    WB_KEY_9 = 57,
    WB_KEY_SEMICOLON = 59,
    WB_KEY_EQUAL = 61,
    WB_KEY_A = 65,
    WB_KEY_B = 66,
    WB_KEY_C = 67,
    WB_KEY_D = 68,
    WB_KEY_E = 69,
    WB_KEY_F = 70,
    WB_KEY_G = 71,
    WB_KEY_H = 72,
    WB_KEY_I = 73,
    WB_KEY_J = 74,
    WB_KEY_K = 75,
    WB_KEY_L = 76,
    WB_KEY_M = 77,
    WB_KEY_N = 78,
    WB_KEY_O = 79,
    WB_KEY_P = 80,
    WB_KEY_Q = 81,
    WB_KEY_R = 82,
    WB_KEY_S = 83,
    WB_KEY_T = 84,
    WB_KEY_U = 85,
    WB_KEY_V = 86,
    WB_KEY_W = 87,
    WB_KEY_X = 88,
    WB_KEY_Y = 89,
    WB_KEY_Z = 90,
    WB_KEY_LEFT_BRACKET = 91,
    WB_KEY_BACKSLASH = 92,
    WB_KEY_RIGHT_BRACKET = 93,
    WB_KEY_GRAVE_ACCENT = 96,
    WB_KEY_WORLD_1 = 161,
    WB_KEY_WORLD_2 = 162,

    WB_KEY_PRINTABLE_COUNT,
    // End of printable keys

    WB_KEY_ESCAPE,
    WB_KEY_ENTER,
    WB_KEY_TAB,
    WB_KEY_BACKSPACE,
    WB_KEY_INSERT,
    WB_KEY_DELETE,
    WB_KEY_RIGHT,
    WB_KEY_LEFT,
    WB_KEY_DOWN,
    WB_KEY_UP,
    WB_KEY_PAGE_UP,
    WB_KEY_PAGE_DOWN,
    WB_KEY_HOME,
    WB_KEY_END,
    WB_KEY_CAPS_LOCK,
    WB_KEY_SCROLL_LOCK,
    WB_KEY_NUM_LOCK,
    WB_KEY_PRINT_SCREEN,
    WB_KEY_PAUSE,
    WB_KEY_F1,
    WB_KEY_F2,
    WB_KEY_F3,
    WB_KEY_F4,
    WB_KEY_F5,
    WB_KEY_F6,
    WB_KEY_F7,
    WB_KEY_F8,
    WB_KEY_F9,
    WB_KEY_F10,
    WB_KEY_F11,
    WB_KEY_F12,
    WB_KEY_F13,
    WB_KEY_F14,
    WB_KEY_F15,
    WB_KEY_F16,
    WB_KEY_F17,
    WB_KEY_F18,
    WB_KEY_F19,
    WB_KEY_F20,
    WB_KEY_F21,
    WB_KEY_F22,
    WB_KEY_F23,
    WB_KEY_F24,
    WB_KEY_F25,
    WB_KEY_KP_0,
    WB_KEY_KP_1,
    WB_KEY_KP_2,
    WB_KEY_KP_3,
    WB_KEY_KP_4,
    WB_KEY_KP_5,
    WB_KEY_KP_6,
    WB_KEY_KP_7,
    WB_KEY_KP_8,
    WB_KEY_KP_9,
    WB_KEY_KP_DECIMAL,
    WB_KEY_KP_DIVIDE,
    WB_KEY_KP_MULTIPLY,
    WB_KEY_KP_SUBTRACT,
    WB_KEY_KP_ADD,
    WB_KEY_KP_ENTER,
    WB_KEY_KP_EQUAL,
    WB_KEY_LEFT_SHIFT,
    WB_KEY_LEFT_CONTROL,
    WB_KEY_LEFT_ALT,
    WB_KEY_LEFT_SUPER,
    WB_KEY_RIGHT_SHIFT,
    WB_KEY_RIGHT_CONTROL,
    WB_KEY_RIGHT_ALT,
    WB_KEY_RIGHT_SUPER,
    WB_KEY_MENU,

    WB_KEY_COUNT
} wb_key;

typedef enum
{
    WB_KEY_MOD_NONE = 0x0000,
    WB_KEY_MOD_LSHIFT = 0x0001,
    WB_KEY_MOD_RSHIFT = 0x0002,
    WB_KEY_MOD_LCTRL = 0x0040,
    WB_KEY_MOD_RCTRL = 0x0080,
    WB_KEY_MOD_LALT = 0x0100,
    WB_KEY_MOD_RALT = 0x0200,
    WB_KEY_MOD_LGUI = 0x0400,
    WB_KEY_MOD_RGUI = 0x0800,
    WB_KEY_MOD_NUM = 0x1000,
    WB_KEY_MOD_CAPS = 0x2000,
    WB_KEY_MOD_MODE = 0x4000,
    WB_KEY_MOD_RESERVED = 0x8000,

    WB_KEY_MOD_CTRL = WB_KEY_MOD_LCTRL | WB_KEY_MOD_RCTRL,
	WB_KEY_MOD_SHIFT = WB_KEY_MOD_LSHIFT | WB_KEY_MOD_RSHIFT,
	WB_KEY_MOD_ALT = WB_KEY_MOD_LALT | WB_KEY_MOD_RALT,
	WB_KEY_MOD_GUI = WB_KEY_MOD_LGUI | WB_KEY_MOD_RGUI
} wb_key_mod;

void wb_input_update(uint16_t key_mod_state);
bool wb_input_handle_key(uint8_t key, bool is_up);
void wb_input_handle_mouse_button(uint8_t mouse_button, bool is_up);
void wb_input_handle_mouse_motion(int32_t x, int32_t y);
void wb_input_handle_mouse_wheel(int32_t x, int32_t y);

uint16_t wb_mouse_x(void);
uint16_t wb_mouse_y(void);
void wb_mouse_position(uint16_t* x, uint16_t* y);
bool wb_mouse_down(wb_mouse_button mouse_button);
bool wb_mouse_up(wb_mouse_button mouse_button);
bool wb_mouse_pressed(wb_mouse_button mouse_button);
bool wb_mouse_released(wb_mouse_button mouse_button);
bool wb_mouse_double_clicked(void);
bool wb_mouse_drag(uint16_t* start_x, uint16_t* start_y);

bool wb_key_down(wb_key key);
bool wb_key_up(wb_key key);
bool wb_key_pressed(wb_key key);
bool wb_key_released(wb_key key);