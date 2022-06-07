#include "input.h"
#include "bits.inl"
#include <string.h>

static wb_bitset256 keys_bitset;
static wb_bitset256 previous_keys_bitset;
static wb_bitset8 mouse_bitset;
static wb_bitset8 previous_mouse_bitset;

static u16 key_mods;

static u16 mouse_x;
static u16 mouse_y;
static s32 mouse_delta_x;
static s32 mouse_delta_y;
static s32 mouse_wheel_x;
static s32 mouse_wheel_y;

//static bool mouse_double_click;
//static u32 last_click_tick;

//static char unicode_chars[KEY_PRINTABLE_COUNT];

void wb_input_update(u16 key_mod_state)
{
	key_mods = key_mod_state;

	memcpy(&previous_keys_bitset, &keys_bitset, sizeof(keys_bitset));

	previous_mouse_bitset = mouse_bitset;
	mouse_delta_x = mouse_delta_y = mouse_wheel_x = mouse_wheel_y = 0;
}

bool wb_input_handle_key(u8 key, bool is_up)
{
	if (is_up)
		wb_bits256_clear(&keys_bitset, key);
	else
		wb_bits256_set(&keys_bitset, key);

#ifdef _WIN32
	// Quit on Alt-F4
	if (wb_key_pressed(WB_KEY_F4) && wb_is_flag_set(key_mods, WB_KEY_MOD_LALT))
	{
		return false;
	}
#endif
	return true;
}

void wb_input_handle_mouse_button(u8 mouse_button, bool is_up)
{
	if (is_up)
		wb_bits8_clear(&mouse_bitset, mouse_button - 1);
	else
		wb_bits8_set(&mouse_bitset, mouse_button - 1);
}

void wb_input_handle_mouse_motion(s32 x, s32 y)
{
	mouse_delta_x += x;
	mouse_delta_y += y;
}

void wb_input_handle_mouse_wheel(s32 x, s32 y)
{
	mouse_wheel_x += x;
	mouse_wheel_y += y;
}

u16 wb_mouse_x(void)
{
	return mouse_x;
}

u16 wb_mouse_y(void)
{
	return mouse_y;
}

void wb_mouse_position(u16* x, u16* y)
{
	*x = mouse_x;
	*y = mouse_y;
}

bool wb_mouse_down(wb_mouse_button mouse_button)
{
    return wb_bits8_is_set(mouse_bitset, mouse_button);
}

bool wb_mouse_up(wb_mouse_button mouse_button)
{
    return !wb_bits8_is_set(mouse_bitset, mouse_button);
}

bool wb_mouse_pressed(wb_mouse_button mouse_button)
{
    return wb_bits8_is_set(mouse_bitset, mouse_button) &&
           !wb_bits8_is_set(previous_mouse_bitset, mouse_button);
}

bool wb_mouse_released(wb_mouse_button mouse_button)
{
    return !wb_bits8_is_set(mouse_bitset, mouse_button) &&
           wb_bits8_is_set(previous_mouse_bitset, mouse_button);
}

bool wb_mouse_double_clicked(void)
{
    return 0;
}

bool wb_mouse_drag(u16* start_x, u16* start_y)
{
	return false;
}

bool wb_key_down(wb_key key)
{
    return wb_bits256_is_set(&keys_bitset, key);
}

bool wb_key_up(wb_key key)
{
    return !wb_bits256_is_set(&keys_bitset, key);
}

bool wb_key_pressed(wb_key key)
{
    return wb_bits256_is_set(&keys_bitset, key) && !wb_bits256_is_set(&previous_keys_bitset, key);
}

bool wb_key_released(wb_key key)
{
    return !wb_bits256_is_set(&keys_bitset, key) && wb_bits256_is_set(&previous_keys_bitset, key);
}