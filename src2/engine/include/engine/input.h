#pragma once

#include <core/types.h>

#define WC_ENUM(K, V) case WC_##K = V,
#define WC_MOUSE_BUTTON_DEFS                                                                                                     \
	WC_ENUM(MOUSE_BUTTON_LEFT, 0)                                                                                                \
	WC_ENUM(MOUSE_BUTTON_RIGHT, 1)                                                                                               \
	WC_ENUM(MOUSE_BUTTON_MIDDLE, 2)                                                                                              \
	WC_ENUM(MOUSE_BUTTON_X1, 3)                                                                                                  \
	WC_ENUM(MOUSE_BUTTON_X2, 4)                                                                                                  \
	WC_ENUM(MOUSE_BUTTON_COUNT, 5)
#undef WC_ENUM

typedef enum WC_MouseButton
{
#define WC_ENUM(K, V) WC_##K = V,
	WC_MOUSE_BUTTON_DEFS
#undef WC_ENUM
} WC_MouseButton;

static const char* wc_mouse_button_to_string(const WC_MouseButton button)
{
	switch (button)
	{
#define WC_ENUM(K, V)                                                                                                            \
	case WC_##K:                                                                                                                 \
		return RG_STRINGIFY(WC_##K);
		WC_MOUSE_BUTTON_DEFS
#undef WC_ENUM
		default:
			return NULL;
	}
}

#define WC_ENUM(K, V) case WC_##K = V,
#define WC_KEY_BUTTON_DEFS                                                                                                       \
	WC_ENUM(KEY_UNKNOWN, 0)                                                                                                      \
	WC_ENUM(KEY_RETURN, 13)                                                                                                      \
	WC_ENUM(KEY_ESCAPE, '\033')                                                                                                  \
	WC_ENUM(KEY_BACKSPACE, '\b')                                                                                                 \
	WC_ENUM(KEY_TAB, '\t')                                                                                                       \
	WC_ENUM(KEY_SPACE, ' ')                                                                                                      \
	WC_ENUM(KEY_EXCLAIM, '!')                                                                                                    \
	WC_ENUM(KEY_QUOTEDBL, '"')                                                                                                   \
	WC_ENUM(KEY_HASH, '#')                                                                                                       \
	WC_ENUM(KEY_PERCENT, '%')                                                                                                    \
	WC_ENUM(KEY_DOLLAR, '$')                                                                                                     \
	WC_ENUM(KEY_AMPERSAND, '&')                                                                                                  \
	WC_ENUM(KEY_QUOTE, '\'')                                                                                                     \
	WC_ENUM(KEY_LEFTPAREN, '(')                                                                                                  \
	WC_ENUM(KEY_RIGHTPAREN, ')')                                                                                                 \
	WC_ENUM(KEY_ASTERISK, '*')                                                                                                   \
	WC_ENUM(KEY_PLUS, '+')                                                                                                       \
	WC_ENUM(KEY_COMMA, ',')                                                                                                      \
	WC_ENUM(KEY_MINUS, '-')                                                                                                      \
	WC_ENUM(KEY_PERIOD, '.')                                                                                                     \
	WC_ENUM(KEY_SLASH, '/')                                                                                                      \
	WC_ENUM(KEY_0, '0')                                                                                                          \
	WC_ENUM(KEY_1, '1')                                                                                                          \
	WC_ENUM(KEY_2, '2')                                                                                                          \
	WC_ENUM(KEY_3, '3')                                                                                                          \
	WC_ENUM(KEY_4, '4')                                                                                                          \
	WC_ENUM(KEY_5, '5')                                                                                                          \
	WC_ENUM(KEY_6, '6')                                                                                                          \
	WC_ENUM(KEY_7, '7')                                                                                                          \
	WC_ENUM(KEY_8, '8')                                                                                                          \
	WC_ENUM(KEY_9, '9')                                                                                                          \
	WC_ENUM(KEY_COLON, ':')                                                                                                      \
	WC_ENUM(KEY_SEMICOLON, ';')                                                                                                  \
	WC_ENUM(KEY_LESS, '<')                                                                                                       \
	WC_ENUM(KEY_EQUALS, '=')                                                                                                     \
	WC_ENUM(KEY_GREATER, '>')                                                                                                    \
	WC_ENUM(KEY_QUESTION, '?')                                                                                                   \
	WC_ENUM(KEY_AT, '@')                                                                                                         \
	WC_ENUM(KEY_LEFTBRACKET, '[')                                                                                                \
	WC_ENUM(KEY_BACKSLASH, '\\')                                                                                                 \
	WC_ENUM(KEY_RIGHTBRACKET, ']')                                                                                               \
	WC_ENUM(KEY_CARET, '^')                                                                                                      \
	WC_ENUM(KEY_UNDERSCORE, '_')                                                                                                 \
	WC_ENUM(KEY_BACKQUOTE, '`')                                                                                                  \
	WC_ENUM(KEY_A, 'a')                                                                                                          \
	WC_ENUM(KEY_B, 'b')                                                                                                          \
	WC_ENUM(KEY_C, 'c')                                                                                                          \
	WC_ENUM(KEY_D, 'd')                                                                                                          \
	WC_ENUM(KEY_E, 'e')                                                                                                          \
	WC_ENUM(KEY_F, 'f')                                                                                                          \
	WC_ENUM(KEY_G, 'g')                                                                                                          \
	WC_ENUM(KEY_H, 'h')                                                                                                          \
	WC_ENUM(KEY_I, 'i')                                                                                                          \
	WC_ENUM(KEY_J, 'j')                                                                                                          \
	WC_ENUM(KEY_K, 'k')                                                                                                          \
	WC_ENUM(KEY_L, 'l')                                                                                                          \
	WC_ENUM(KEY_M, 'm')                                                                                                          \
	WC_ENUM(KEY_N, 'n')                                                                                                          \
	WC_ENUM(KEY_O, 'o')                                                                                                          \
	WC_ENUM(KEY_P, 'p')                                                                                                          \
	WC_ENUM(KEY_Q, 'q')                                                                                                          \
	WC_ENUM(KEY_R, 'r')                                                                                                          \
	WC_ENUM(KEY_S, 's')                                                                                                          \
	WC_ENUM(KEY_T, 't')                                                                                                          \
	WC_ENUM(KEY_U, 'u')                                                                                                          \
	WC_ENUM(KEY_V, 'v')                                                                                                          \
	WC_ENUM(KEY_W, 'w')                                                                                                          \
	WC_ENUM(KEY_X, 'x')                                                                                                          \
	WC_ENUM(KEY_Y, 'y')                                                                                                          \
	WC_ENUM(KEY_Z, 'z')                                                                                                          \
	WC_ENUM(KEY_CAPSLOCK, 123)                                                                                                   \
	WC_ENUM(KEY_F1, 124)                                                                                                         \
	WC_ENUM(KEY_F2, 125)                                                                                                         \
	WC_ENUM(KEY_F3, 126)                                                                                                         \
	WC_ENUM(KEY_F4, 127)                                                                                                         \
	WC_ENUM(KEY_F5, 128)                                                                                                         \
	WC_ENUM(KEY_F6, 129)                                                                                                         \
	WC_ENUM(KEY_F7, 130)                                                                                                         \
	WC_ENUM(KEY_F8, 131)                                                                                                         \
	WC_ENUM(KEY_F9, 132)                                                                                                         \
	WC_ENUM(KEY_F10, 133)                                                                                                        \
	WC_ENUM(KEY_F11, 134)                                                                                                        \
	WC_ENUM(KEY_F12, 135)                                                                                                        \
	WC_ENUM(KEY_PRINTSCREEN, 136)                                                                                                \
	WC_ENUM(KEY_SCROLLLOCK, 137)                                                                                                 \
	WC_ENUM(KEY_PAUSE, 138)                                                                                                      \
	WC_ENUM(KEY_INSERT, 139)                                                                                                     \
	WC_ENUM(KEY_HOME, 140)                                                                                                       \
	WC_ENUM(KEY_PAGEUP, 141)                                                                                                     \
	WC_ENUM(KEY_DELETE, 142)                                                                                                     \
	WC_ENUM(KEY_END, 143)                                                                                                        \
	WC_ENUM(KEY_PAGEDOWN, 144)                                                                                                   \
	WC_ENUM(KEY_RIGHT, 145)                                                                                                      \
	WC_ENUM(KEY_LEFT, 146)                                                                                                       \
	WC_ENUM(KEY_DOWN, 147)                                                                                                       \
	WC_ENUM(KEY_UP, 148)                                                                                                         \
	WC_ENUM(KEY_NUMLOCKCLEAR, 149)                                                                                               \
	WC_ENUM(KEY_KP_DIVIDE, 150)                                                                                                  \
	WC_ENUM(KEY_KP_MULTIPLY, 151)                                                                                                \
	WC_ENUM(KEY_KP_MINUS, 152)                                                                                                   \
	WC_ENUM(KEY_KP_PLUS, 153)                                                                                                    \
	WC_ENUM(KEY_KP_ENTER, 154)                                                                                                   \
	WC_ENUM(KEY_KP_1, 155)                                                                                                       \
	WC_ENUM(KEY_KP_2, 156)                                                                                                       \
	WC_ENUM(KEY_KP_3, 157)                                                                                                       \
	WC_ENUM(KEY_KP_4, 158)                                                                                                       \
	WC_ENUM(KEY_KP_5, 159)                                                                                                       \
	WC_ENUM(KEY_KP_6, 160)                                                                                                       \
	WC_ENUM(KEY_KP_7, 161)                                                                                                       \
	WC_ENUM(KEY_KP_8, 162)                                                                                                       \
	WC_ENUM(KEY_KP_9, 163)                                                                                                       \
	WC_ENUM(KEY_KP_0, 164)                                                                                                       \
	WC_ENUM(KEY_KP_PERIOD, 165)                                                                                                  \
	WC_ENUM(KEY_APPLICATION, 166)                                                                                                \
	WC_ENUM(KEY_POWER, 167)                                                                                                      \
	WC_ENUM(KEY_KP_EQUALS, 168)                                                                                                  \
	WC_ENUM(KEY_F13, 169)                                                                                                        \
	WC_ENUM(KEY_F14, 170)                                                                                                        \
	WC_ENUM(KEY_F15, 171)                                                                                                        \
	WC_ENUM(KEY_F16, 172)                                                                                                        \
	WC_ENUM(KEY_F17, 173)                                                                                                        \
	WC_ENUM(KEY_F18, 174)                                                                                                        \
	WC_ENUM(KEY_F19, 175)                                                                                                        \
	WC_ENUM(KEY_F20, 176)                                                                                                        \
	WC_ENUM(KEY_F21, 177)                                                                                                        \
	WC_ENUM(KEY_F22, 178)                                                                                                        \
	WC_ENUM(KEY_F23, 179)                                                                                                        \
	WC_ENUM(KEY_F24, 180)                                                                                                        \
	WC_ENUM(KEY_HELP, 181)                                                                                                       \
	WC_ENUM(KEY_MENU, 182)                                                                                                       \
	WC_ENUM(KEY_SELECT, 183)                                                                                                     \
	WC_ENUM(KEY_STOP, 184)                                                                                                       \
	WC_ENUM(KEY_AGAIN, 185)                                                                                                      \
	WC_ENUM(KEY_UNDO, 186)                                                                                                       \
	WC_ENUM(KEY_CUT, 187)                                                                                                        \
	WC_ENUM(KEY_COPY, 188)                                                                                                       \
	WC_ENUM(KEY_PASTE, 189)                                                                                                      \
	WC_ENUM(KEY_FIND, 190)                                                                                                       \
	WC_ENUM(KEY_MUTE, 191)                                                                                                       \
	WC_ENUM(KEY_VOLUMEUP, 192)                                                                                                   \
	WC_ENUM(KEY_VOLUMEDOWN, 193)                                                                                                 \
	WC_ENUM(KEY_KP_COMMA, 194)                                                                                                   \
	WC_ENUM(KEY_KP_EQUALSAS400, 195)                                                                                             \
	WC_ENUM(KEY_ALTERASE, 196)                                                                                                   \
	WC_ENUM(KEY_SYSREQ, 197)                                                                                                     \
	WC_ENUM(KEY_CANCEL, 198)                                                                                                     \
	WC_ENUM(KEY_CLEAR, 199)                                                                                                      \
	WC_ENUM(KEY_PRIOR, 200)                                                                                                      \
	WC_ENUM(KEY_RETURN2, 201)                                                                                                    \
	WC_ENUM(KEY_SEPARATOR, 202)                                                                                                  \
	WC_ENUM(KEY_OUT, 203)                                                                                                        \
	WC_ENUM(KEY_OPER, 204)                                                                                                       \
	WC_ENUM(KEY_CLEARAGAIN, 205)                                                                                                 \
	WC_ENUM(KEY_CRSEL, 206)                                                                                                      \
	WC_ENUM(KEY_EXSEL, 207)                                                                                                      \
	WC_ENUM(KEY_KP_00, 208)                                                                                                      \
	WC_ENUM(KEY_KP_000, 209)                                                                                                     \
	WC_ENUM(KEY_THOUSANDSSEPARATOR, 210)                                                                                         \
	WC_ENUM(KEY_DECIMALSEPARATOR, 211)                                                                                           \
	WC_ENUM(KEY_CURRENCYUNIT, 212)                                                                                               \
	WC_ENUM(KEY_CURRENCYSUBUNIT, 213)                                                                                            \
	WC_ENUM(KEY_KP_LEFTPAREN, 214)                                                                                               \
	WC_ENUM(KEY_KP_RIGHTPAREN, 215)                                                                                              \
	WC_ENUM(KEY_KP_LEFTBRACE, 216)                                                                                               \
	WC_ENUM(KEY_KP_RIGHTBRACE, 217)                                                                                              \
	WC_ENUM(KEY_KP_TAB, 218)                                                                                                     \
	WC_ENUM(KEY_KP_BACKSPACE, 219)                                                                                               \
	WC_ENUM(KEY_KP_A, 220)                                                                                                       \
	WC_ENUM(KEY_KP_B, 221)                                                                                                       \
	WC_ENUM(KEY_KP_C, 222)                                                                                                       \
	WC_ENUM(KEY_KP_D, 223)                                                                                                       \
	WC_ENUM(KEY_KP_E, 224)                                                                                                       \
	WC_ENUM(KEY_KP_F, 225)                                                                                                       \
	WC_ENUM(KEY_KP_XOR, 226)                                                                                                     \
	WC_ENUM(KEY_KP_POWER, 227)                                                                                                   \
	WC_ENUM(KEY_KP_PERCENT, 228)                                                                                                 \
	WC_ENUM(KEY_KP_LESS, 229)                                                                                                    \
	WC_ENUM(KEY_KP_GREATER, 230)                                                                                                 \
	WC_ENUM(KEY_KP_AMPERSAND, 231)                                                                                               \
	WC_ENUM(KEY_KP_DBLAMPERSAND, 232)                                                                                            \
	WC_ENUM(KEY_KP_VERTICALBAR, 233)                                                                                             \
	WC_ENUM(KEY_KP_DBLVERTICALBAR, 234)                                                                                          \
	WC_ENUM(KEY_KP_COLON, 235)                                                                                                   \
	WC_ENUM(KEY_KP_HASH, 236)                                                                                                    \
	WC_ENUM(KEY_KP_SPACE, 237)                                                                                                   \
	WC_ENUM(KEY_KP_AT, 238)                                                                                                      \
	WC_ENUM(KEY_KP_EXCLAM, 239)                                                                                                  \
	WC_ENUM(KEY_KP_MEMSTORE, 240)                                                                                                \
	WC_ENUM(KEY_KP_MEMRECALL, 241)                                                                                               \
	WC_ENUM(KEY_KP_MEMCLEAR, 242)                                                                                                \
	WC_ENUM(KEY_KP_MEMADD, 243)                                                                                                  \
	WC_ENUM(KEY_KP_MEMSUBTRACT, 244)                                                                                             \
	WC_ENUM(KEY_KP_MEMMULTIPLY, 245)                                                                                             \
	WC_ENUM(KEY_KP_MEMDIVIDE, 246)                                                                                               \
	WC_ENUM(KEY_KP_PLUSMINUS, 247)                                                                                               \
	WC_ENUM(KEY_KP_CLEAR, 248)                                                                                                   \
	WC_ENUM(KEY_KP_CLEARENTRY, 249)                                                                                              \
	WC_ENUM(KEY_KP_BINARY, 250)                                                                                                  \
	WC_ENUM(KEY_KP_OCTAL, 251)                                                                                                   \
	WC_ENUM(KEY_KP_DECIMAL, 252)                                                                                                 \
	WC_ENUM(KEY_KP_HEXADECIMAL, 253)                                                                                             \
	WC_ENUM(KEY_LCTRL, 254)                                                                                                      \
	WC_ENUM(KEY_LSHIFT, 255)                                                                                                     \
	WC_ENUM(KEY_LALT, 256)                                                                                                       \
	WC_ENUM(KEY_LGUI, 257)                                                                                                       \
	WC_ENUM(KEY_RCTRL, 258)                                                                                                      \
	WC_ENUM(KEY_RSHIFT, 259)                                                                                                     \
	WC_ENUM(KEY_RALT, 260)                                                                                                       \
	WC_ENUM(KEY_RGUI, 261)                                                                                                       \
	WC_ENUM(KEY_MODE, 262)                                                                                                       \
	WC_ENUM(KEY_AUDIONEXT, 263)                                                                                                  \
	WC_ENUM(KEY_AUDIOPREV, 264)                                                                                                  \
	WC_ENUM(KEY_AUDIOSTOP, 265)                                                                                                  \
	WC_ENUM(KEY_AUDIOPLAY, 266)                                                                                                  \
	WC_ENUM(KEY_AUDIOMUTE, 267)                                                                                                  \
	WC_ENUM(KEY_MEDIASELECT, 268)                                                                                                \
	WC_ENUM(KEY_WWW, 269)                                                                                                        \
	WC_ENUM(KEY_MAIL, 270)                                                                                                       \
	WC_ENUM(KEY_CALCULATOR, 271)                                                                                                 \
	WC_ENUM(KEY_COMPUTER, 272)                                                                                                   \
	WC_ENUM(KEY_AC_SEARCH, 273)                                                                                                  \
	WC_ENUM(KEY_AC_HOME, 274)                                                                                                    \
	WC_ENUM(KEY_AC_BACK, 275)                                                                                                    \
	WC_ENUM(KEY_AC_FORWARD, 276)                                                                                                 \
	WC_ENUM(KEY_AC_STOP, 277)                                                                                                    \
	WC_ENUM(KEY_AC_REFRESH, 278)                                                                                                 \
	WC_ENUM(KEY_AC_BOOKMARKS, 279)                                                                                               \
	WC_ENUM(KEY_BRIGHTNESSDOWN, 280)                                                                                             \
	WC_ENUM(KEY_BRIGHTNESSUP, 281)                                                                                               \
	WC_ENUM(KEY_DISPLAYSWITCH, 282)                                                                                              \
	WC_ENUM(KEY_KBDILLUMTOGGLE, 283)                                                                                             \
	WC_ENUM(KEY_KBDILLUMDOWN, 284)                                                                                               \
	WC_ENUM(KEY_KBDILLUMUP, 285)                                                                                                 \
	WC_ENUM(KEY_EJECT, 286)                                                                                                      \
	WC_ENUM(KEY_SLEEP, 287)                                                                                                      \
	WC_ENUM(KEY_ANY, 288)                                                                                                        \
	WC_ENUM(KEY_COUNT, 512)
#undef WC_ENUM

typedef enum WC_KeyButton
{
#define WC_ENUM(K, V) WC_##K = V,
	WC_KEY_BUTTON_DEFS
#undef WC_ENUM
} WC_KeyButton;

static const char* wc_key_button_to_string(const WC_KeyButton button)
{
	switch (button)
	{
#define WC_ENUM(K, V)                                                                                                            \
	case WC_##K:                                                                                                                 \
		return RG_STRINGIFY(WC_##K);
		WC_KEY_BUTTON_DEFS
#undef WC_ENUM
		default:
			return NULL;
	}
}

bool wc_key_down(WC_KeyButton key);
bool wc_key_up(WC_KeyButton key);
bool wc_key_just_pressed(WC_KeyButton key);
bool wc_key_just_released(WC_KeyButton key);
bool wc_key_repeating(WC_KeyButton key);
bool wc_key_ctrl();
bool wc_key_shift();
bool wc_key_alt();
bool wc_key_gui();

void wc_clear_key_states();

void wc_register_key_callback(void (*key_callback)(WC_KeyButton key, bool true_down_false_up));

float wc_mouse_x();
float wc_mouse_y();
float wc_mouse_motion_x();
float wc_mouse_motion_y();
bool wc_mouse_down(WC_MouseButton button);
bool wc_mouse_just_pressed(WC_MouseButton button);
bool wc_mouse_just_released(WC_MouseButton button);
float wc_mouse_wheel_motion();
bool wc_mouse_double_click_held(WC_MouseButton button);
bool wc_mouse_double_clicked(WC_MouseButton button);
void wc_mouse_hide(bool true_to_hide);
bool wc_mouse_hidden();
void wc_mouse_lock_inside_window(bool true_to_lock);
void wc_mouse_set_relative_mode(bool true_to_set_relative);

typedef struct WC_InputTextBuffer
{
	int len;
	const int* codepoints;
} WC_InputTextBuffer;

void wc_input_text_add_utf8(const char* text);
int wc_input_text_pop_utf32();
bool wc_input_text_has_data();
bool wc_input_text_get_buffer(WC_InputTextBuffer* buffer);
void wc_input_text_clear();
void wc_input_enable_ime();
void wc_input_disable_ime();
bool wc_input_is_ime_enabled();
bool wc_input_has_ime_keyboard_support();
bool wc_input_is_ime_keyboard_shown();
void wc_input_set_ime_rect(int x, int y, int w, int h);

typedef struct WC_ImeComposition
{
	const char* composition;
	int cursor;
	int selection_len;
} WC_ImeComposition;

bool wc_input_get_ime_composition(WC_ImeComposition* composition);
