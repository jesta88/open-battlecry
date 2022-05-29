#include "../engine/engine.h"
#include "../engine/input.h"
#include "../engine/log.h"
#include "../engine/bits.inl"
#include <SDL.h>
#include <assert.h>

enum
{
	MAX_WINDOW_EVENTS = 8,
	MAX_KEYBOARD_EVENTS = 64,
	MAX_MOUSE_EVENTS = 256,
};

static const uint8_t scancode_to_key[] = {
		0, 0, 0, 0, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
		KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_1, KEY_2,
		KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_ENTER, KEY_ESCAPE, KEY_BACKSPACE, KEY_TAB,
		KEY_SPACE, KEY_MINUS, KEY_EQUAL, KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET, KEY_BACKSLASH, 0, KEY_SEMICOLON,
		KEY_APOSTROPHE, KEY_GRAVE_ACCENT, KEY_COMMA, KEY_PERIOD, KEY_SLASH, KEY_CAPS_LOCK, KEY_F1, KEY_F2, KEY_F3,
		KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_PRINT_SCREEN, KEY_SCROLL_LOCK, KEY_PAUSE,
		KEY_INSERT, KEY_HOME, KEY_PAGE_UP, KEY_DELETE, KEY_END, KEY_PAGE_DOWN, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP,
		KEY_NUM_LOCK, KEY_KP_DIVIDE, KEY_KP_MULTIPLY, KEY_KP_SUBTRACT, KEY_KP_ADD, KEY_KP_ENTER, KEY_KP_1, KEY_KP_2, KEY_KP_3,
		KEY_KP_4, KEY_KP_5, KEY_KP_6, KEY_KP_7, KEY_KP_8, KEY_KP_9, KEY_KP_0, KEY_KP_DECIMAL, 0, 0, 0, KEY_KP_EQUAL};

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);

	bool borderless = false;
	bool fullscreen = false;

	uint32_t window_flags = SDL_WINDOW_ALLOW_HIGHDPI;
	if (fullscreen && borderless) 
		window_flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN_DESKTOP;
	else if (fullscreen)
		window_flags |= SDL_WINDOW_FULLSCREEN;

	SDL_Window* window = SDL_CreateWindow(
			"Battlecry", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			1280, 720, window_flags);
	assert(window != NULL);

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

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
