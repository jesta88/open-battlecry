#include "graphics.h"
#include "../core/engine.h"
#include "../core/input.h"
#include "../core/log.h"
#include "../core/bits.inl"
#include <SDL_events.h>
#include <assert.h>

enum
{
	MAX_WINDOW_EVENTS = 8,
	MAX_KEYBOARD_EVENTS = 64,
	MAX_MOUSE_EVENTS = 256,
};

static const uint8_t scancode_to_key[] = {
		0, 0, 0, 0, WB_KEY_A, WB_KEY_B, WB_KEY_C, WB_KEY_D, WB_KEY_E, WB_KEY_F, WB_KEY_G, WB_KEY_H, WB_KEY_I, WB_KEY_J, WB_KEY_K, WB_KEY_L, WB_KEY_M,
		WB_KEY_N, WB_KEY_O, WB_KEY_P, WB_KEY_Q, WB_KEY_R, WB_KEY_S, WB_KEY_T, WB_KEY_U, WB_KEY_V, WB_KEY_W, WB_KEY_X, WB_KEY_Y, WB_KEY_Z, WB_KEY_1, WB_KEY_2,
		WB_KEY_3, WB_KEY_4, WB_KEY_5, WB_KEY_6, WB_KEY_7, WB_KEY_8, WB_KEY_9, WB_KEY_0, WB_KEY_ENTER, WB_KEY_ESCAPE, WB_KEY_BACKSPACE, WB_KEY_TAB,
		WB_KEY_SPACE, WB_KEY_MINUS, WB_KEY_EQUAL, WB_KEY_LEFT_BRACKET, WB_KEY_RIGHT_BRACKET, WB_KEY_BACKSLASH, 0, WB_KEY_SEMICOLON,
		WB_KEY_APOSTROPHE, WB_KEY_GRAVE_ACCENT, WB_KEY_COMMA, WB_KEY_PERIOD, WB_KEY_SLASH, WB_KEY_CAPS_LOCK, WB_KEY_F1, WB_KEY_F2, WB_KEY_F3,
		WB_KEY_F4, WB_KEY_F5, WB_KEY_F6, WB_KEY_F7, WB_KEY_F8, WB_KEY_F9, WB_KEY_F10, WB_KEY_F11, WB_KEY_F12, WB_KEY_PRINT_SCREEN, WB_KEY_SCROLL_LOCK, WB_KEY_PAUSE,
		WB_KEY_INSERT, WB_KEY_HOME, WB_KEY_PAGE_UP, WB_KEY_DELETE, WB_KEY_END, WB_KEY_PAGE_DOWN, WB_KEY_RIGHT, WB_KEY_LEFT, WB_KEY_DOWN, WB_KEY_UP,
		WB_KEY_NUM_LOCK, WB_KEY_KP_DIVIDE, WB_KEY_KP_MULTIPLY, WB_KEY_KP_SUBTRACT, WB_KEY_KP_ADD, WB_KEY_KP_ENTER, WB_KEY_KP_1, WB_KEY_KP_2, WB_KEY_KP_3,
		WB_KEY_KP_4, WB_KEY_KP_5, WB_KEY_KP_6, WB_KEY_KP_7, WB_KEY_KP_8, WB_KEY_KP_9, WB_KEY_KP_0, WB_KEY_KP_DECIMAL, 0, 0, 0, WB_KEY_KP_EQUAL};

int main(int argc, char* argv[])
{
	wb_graphics_init(&(const wb_graphics_desc)
	{
		.window_name = "Battlecry"
	});

	bool quit = false;
	float delta_time = 0.005f;
	uint64_t last_tick = SDL_GetPerformanceCounter();
	SDL_Event event;

	while (!quit)
	{
		wb_input_update(SDL_GetModState());

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				uint8_t key = scancode_to_key[event.key.keysym.scancode];
				if (!wb_input_handle_key(key, event.key.type == SDL_KEYUP))
					quit = true;
				break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
				uint8_t mouse_button = event.button.button;
				wb_input_handle_mouse_button(mouse_button, event.button.type == SDL_MOUSEBUTTONUP);
				break;
			case SDL_MOUSEMOTION:
				wb_input_handle_mouse_motion(event.motion.xrel, event.motion.yrel);
				break;
			case SDL_MOUSEWHEEL:
				wb_input_handle_mouse_wheel(event.wheel.x, event.wheel.y);
				break;
			case SDL_WINDOWEVENT:
				//handleActiveEvent(&event);
				break;
			case SDL_TEXTINPUT:
				//inputhandleText(&event.text);
				break;
			case SDL_QUIT:
				quit = true;
				break;
			default:
				break;
			}
		}

		if (quit)
			continue;

		uint64_t tick = SDL_GetPerformanceCounter();
		uint64_t delta_tick = tick - last_tick;
		last_tick = tick;
		delta_time = (float)(((double)delta_tick * 1000) / (double)SDL_GetPerformanceFrequency());
	}

	wb_graphics_quit();
	return 0;
}
