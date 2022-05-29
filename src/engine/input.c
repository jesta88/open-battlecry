#include "input.h"
#include "bits.inl"
#include <string.h>

static struct bits256_t keys_bitset;
static struct bits256_t previous_keys_bitset;
static struct bits8_t mouse_bitset;
static struct bits8_t previous_mouse_bitset;

static uint16_t key_mods;

static int32_t mouse_delta_x;
static int32_t mouse_delta_y;
static int32_t mouse_wheel_x;
static int32_t mouse_wheel_y;

//static bool mouse_double_click;
//static uint32_t last_click_tick;

//static char unicode_chars[KEY_PRINTABLE_COUNT];

void wb_input_update(uint16_t key_mod_state)
{
	key_mods = key_mod_state;

	memcpy(&previous_keys_bitset, &keys_bitset, sizeof(keys_bitset));

	previous_mouse_bitset = mouse_bitset;
	mouse_delta_x = mouse_delta_y = mouse_wheel_x = mouse_wheel_y = 0;
}

bool wb_input_handle_key(uint8_t key, bool is_up)
{
	if (is_up)
		bits256_clear(&keys_bitset, key);
	else
		bits256_set(&keys_bitset, key);

#ifdef _WIN32
	// Quit on Alt-F4
	if (wb_key_pressed(KEY_F4) && is_flag_set(key_mods, KEY_MOD_LALT))
	{
		return false;
	}
#endif
	return true;
}

void wb_input_handle_mouse_button(uint8_t mouse_button, bool is_up)
{
	if (is_up)
		bits8_clear(&mouse_bitset, mouse_button - 1);
	else
		bits8_set(&mouse_bitset, mouse_button - 1);
}

void wb_input_handle_mouse_motion(int32_t x, int32_t y)
{
	mouse_delta_x += x;
	mouse_delta_y += y;
}

void wb_input_handle_mouse_wheel(int32_t x, int32_t y)
{
	mouse_wheel_x += x;
	mouse_wheel_y += y;
}

bool wb_mouse_down(wb_mouse_button mouse_button)
{
    return bits8_is_set(mouse_bitset, mouse_button);
}

bool wb_mouse_up(wb_mouse_button mouse_button)
{
    return !bits8_is_set(mouse_bitset, mouse_button);
}

bool wb_mouse_pressed(wb_mouse_button mouse_button)
{
    return bits8_is_set(mouse_bitset, mouse_button) &&
           !bits8_is_set(previous_mouse_bitset, mouse_button);
}

bool wb_mouse_released(wb_mouse_button mouse_button)
{
    return !bits8_is_set(mouse_bitset, mouse_button) &&
           bits8_is_set(previous_mouse_bitset, mouse_button);
}

bool wb_mouse_double_pressed(void)
{
    return 0;
}

bool wb_key_down(wb_key key)
{
    return bits256_is_set(&keys_bitset, key);
}

bool wb_key_up(wb_key key)
{
    return !bits256_is_set(&keys_bitset, key);
}

bool wb_key_pressed(wb_key key)
{
    return bits256_is_set(&keys_bitset, key) && !bits256_is_set(&previous_keys_bitset, key);
}

bool wb_key_released(wb_key key)
{
    return !bits256_is_set(&keys_bitset, key) && bits256_is_set(&previous_keys_bitset, key);
}