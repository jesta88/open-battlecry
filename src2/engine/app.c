#include "engine/app.h"

#include "engine/config.h"
#include "engine/input.h"

#include <SDL3/SDL.h>

#define WC_LOGIC_FREQUENCY 60.0
#define WC_FIXED_TIMESTEP (double)(1.0 / WC_LOGIC_FREQUENCY)
#define WC_MAX_ACCUMULATOR 0.25

typedef struct war_window
{
	SDL_Window* handle;
	int x;
	int y;
	int width;
	int height;
	bool fullscreen;
	bool resized;
	bool moved;
	bool minimized;
	bool maximized;
	bool restored;
	bool mouse_inside_window;
	bool has_keyboard_focus;
} war_window_t;

typedef struct war_time
{
	u64 tick;
	u64 tick_previous;
	double tick_frequency;
	double tick_inverse_frequency;
	double accumulator;
	double seconds;
} war_time_t;

typedef struct war_mouse
{
	bool double_click;
	int button_left;
	int button_right;
	int button_middle;
	int button_x1;
	int button_x2;
	int button_x3;
	int button_x4;
	float x;
	float y;
	float delta_x;
	float delta_y;
	float wheel;
} war_mouse_t;

typedef struct war_keyboard
{
	int keys[512];
	int keys_prev[512];
	double keys_timestamp[512];
} war_keyboard_t;

typedef struct war_app
{
	war_window_t window;
	war_time_t time;
	war_mouse_t mouse;
	war_keyboard_t keyboard;

	AppCallbacks callbacks;

	bool running;
} war_app_t;

static int s_map_SDL_keys(const SDL_Keycode key)
{
	if (key < 127)
		return (int)key;
	switch (key)
	{
		case SDLK_CAPSLOCK:
			return WC_KEY_CAPSLOCK;
		case SDLK_F1:
			return WC_KEY_F1;
		case SDLK_F2:
			return WC_KEY_F2;
		case SDLK_F3:
			return WC_KEY_F3;
		case SDLK_F4:
			return WC_KEY_F4;
		case SDLK_F5:
			return WC_KEY_F5;
		case SDLK_F6:
			return WC_KEY_F6;
		case SDLK_F7:
			return WC_KEY_F7;
		case SDLK_F8:
			return WC_KEY_F8;
		case SDLK_F9:
			return WC_KEY_F9;
		case SDLK_F10:
			return WC_KEY_F10;
		case SDLK_F11:
			return WC_KEY_F11;
		case SDLK_F12:
			return WC_KEY_F12;
		case SDLK_PRINTSCREEN:
			return WC_KEY_PRINTSCREEN;
		case SDLK_SCROLLLOCK:
			return WC_KEY_SCROLLLOCK;
		case SDLK_PAUSE:
			return WC_KEY_PAUSE;
		case SDLK_INSERT:
			return WC_KEY_INSERT;
		case SDLK_HOME:
			return WC_KEY_HOME;
		case SDLK_PAGEUP:
			return WC_KEY_PAGEUP;
		case SDLK_DELETE:
			return WC_KEY_DELETE;
		case SDLK_END:
			return WC_KEY_END;
		case SDLK_PAGEDOWN:
			return WC_KEY_PAGEDOWN;
		case SDLK_RIGHT:
			return WC_KEY_RIGHT;
		case SDLK_LEFT:
			return WC_KEY_LEFT;
		case SDLK_DOWN:
			return WC_KEY_DOWN;
		case SDLK_UP:
			return WC_KEY_UP;
		case SDLK_NUMLOCKCLEAR:
			return WC_KEY_NUMLOCKCLEAR;
		case SDLK_KP_DIVIDE:
			return WC_KEY_KP_DIVIDE;
		case SDLK_KP_MULTIPLY:
			return WC_KEY_KP_MULTIPLY;
		case SDLK_KP_MINUS:
			return WC_KEY_KP_MINUS;
		case SDLK_KP_PLUS:
			return WC_KEY_KP_PLUS;
		case SDLK_KP_ENTER:
			return WC_KEY_KP_ENTER;
		case SDLK_KP_1:
			return WC_KEY_KP_1;
		case SDLK_KP_2:
			return WC_KEY_KP_2;
		case SDLK_KP_3:
			return WC_KEY_KP_3;
		case SDLK_KP_4:
			return WC_KEY_KP_4;
		case SDLK_KP_5:
			return WC_KEY_KP_5;
		case SDLK_KP_6:
			return WC_KEY_KP_6;
		case SDLK_KP_7:
			return WC_KEY_KP_7;
		case SDLK_KP_8:
			return WC_KEY_KP_8;
		case SDLK_KP_9:
			return WC_KEY_KP_9;
		case SDLK_KP_0:
			return WC_KEY_KP_0;
		case SDLK_KP_PERIOD:
			return WC_KEY_KP_PERIOD;
		case SDLK_APPLICATION:
			return WC_KEY_APPLICATION;
		case SDLK_POWER:
			return WC_KEY_POWER;
		case SDLK_KP_EQUALS:
			return WC_KEY_KP_EQUALS;
		case SDLK_F13:
			return WC_KEY_F13;
		case SDLK_F14:
			return WC_KEY_F14;
		case SDLK_F15:
			return WC_KEY_F15;
		case SDLK_F16:
			return WC_KEY_F16;
		case SDLK_F17:
			return WC_KEY_F17;
		case SDLK_F18:
			return WC_KEY_F18;
		case SDLK_F19:
			return WC_KEY_F19;
		case SDLK_F20:
			return WC_KEY_F20;
		case SDLK_F21:
			return WC_KEY_F21;
		case SDLK_F22:
			return WC_KEY_F22;
		case SDLK_F23:
			return WC_KEY_F23;
		case SDLK_F24:
			return WC_KEY_F24;
		case SDLK_HELP:
			return WC_KEY_HELP;
		case SDLK_MENU:
			return WC_KEY_MENU;
		case SDLK_SELECT:
			return WC_KEY_SELECT;
		case SDLK_STOP:
			return WC_KEY_STOP;
		case SDLK_AGAIN:
			return WC_KEY_AGAIN;
		case SDLK_UNDO:
			return WC_KEY_UNDO;
		case SDLK_CUT:
			return WC_KEY_CUT;
		case SDLK_COPY:
			return WC_KEY_COPY;
		case SDLK_PASTE:
			return WC_KEY_PASTE;
		case SDLK_FIND:
			return WC_KEY_FIND;
		case SDLK_MUTE:
			return WC_KEY_MUTE;
		case SDLK_VOLUMEUP:
			return WC_KEY_VOLUMEUP;
		case SDLK_VOLUMEDOWN:
			return WC_KEY_VOLUMEDOWN;
		case SDLK_KP_COMMA:
			return WC_KEY_KP_COMMA;
		case SDLK_KP_EQUALSAS400:
			return WC_KEY_KP_EQUALSAS400;
		case SDLK_ALTERASE:
			return WC_KEY_ALTERASE;
		case SDLK_SYSREQ:
			return WC_KEY_SYSREQ;
		case SDLK_CANCEL:
			return WC_KEY_CANCEL;
		case SDLK_CLEAR:
			return WC_KEY_CLEAR;
		case SDLK_PRIOR:
			return WC_KEY_PRIOR;
		case SDLK_RETURN2:
			return WC_KEY_RETURN2;
		case SDLK_SEPARATOR:
			return WC_KEY_SEPARATOR;
		case SDLK_OUT:
			return WC_KEY_OUT;
		case SDLK_OPER:
			return WC_KEY_OPER;
		case SDLK_CLEARAGAIN:
			return WC_KEY_CLEARAGAIN;
		case SDLK_CRSEL:
			return WC_KEY_CRSEL;
		case SDLK_EXSEL:
			return WC_KEY_EXSEL;
		case SDLK_KP_00:
			return WC_KEY_KP_00;
		case SDLK_KP_000:
			return WC_KEY_KP_000;
		case SDLK_THOUSANDSSEPARATOR:
			return WC_KEY_THOUSANDSSEPARATOR;
		case SDLK_DECIMALSEPARATOR:
			return WC_KEY_DECIMALSEPARATOR;
		case SDLK_CURRENCYUNIT:
			return WC_KEY_CURRENCYUNIT;
		case SDLK_CURRENCYSUBUNIT:
			return WC_KEY_CURRENCYSUBUNIT;
		case SDLK_KP_LEFTPAREN:
			return WC_KEY_KP_LEFTPAREN;
		case SDLK_KP_RIGHTPAREN:
			return WC_KEY_KP_RIGHTPAREN;
		case SDLK_KP_LEFTBRACE:
			return WC_KEY_KP_LEFTBRACE;
		case SDLK_KP_RIGHTBRACE:
			return WC_KEY_KP_RIGHTBRACE;
		case SDLK_KP_TAB:
			return WC_KEY_KP_TAB;
		case SDLK_KP_BACKSPACE:
			return WC_KEY_KP_BACKSPACE;
		case SDLK_KP_A:
			return WC_KEY_KP_A;
		case SDLK_KP_B:
			return WC_KEY_KP_B;
		case SDLK_KP_C:
			return WC_KEY_KP_C;
		case SDLK_KP_D:
			return WC_KEY_KP_D;
		case SDLK_KP_E:
			return WC_KEY_KP_E;
		case SDLK_KP_F:
			return WC_KEY_KP_F;
		case SDLK_KP_XOR:
			return WC_KEY_KP_XOR;
		case SDLK_KP_POWER:
			return WC_KEY_KP_POWER;
		case SDLK_KP_PERCENT:
			return WC_KEY_KP_PERCENT;
		case SDLK_KP_LESS:
			return WC_KEY_KP_LESS;
		case SDLK_KP_GREATER:
			return WC_KEY_KP_GREATER;
		case SDLK_KP_AMPERSAND:
			return WC_KEY_KP_AMPERSAND;
		case SDLK_KP_DBLAMPERSAND:
			return WC_KEY_KP_DBLAMPERSAND;
		case SDLK_KP_VERTICALBAR:
			return WC_KEY_KP_VERTICALBAR;
		case SDLK_KP_DBLVERTICALBAR:
			return WC_KEY_KP_DBLVERTICALBAR;
		case SDLK_KP_COLON:
			return WC_KEY_KP_COLON;
		case SDLK_KP_HASH:
			return WC_KEY_KP_HASH;
		case SDLK_KP_SPACE:
			return WC_KEY_KP_SPACE;
		case SDLK_KP_AT:
			return WC_KEY_KP_AT;
		case SDLK_KP_EXCLAM:
			return WC_KEY_KP_EXCLAM;
		case SDLK_KP_MEMSTORE:
			return WC_KEY_KP_MEMSTORE;
		case SDLK_KP_MEMRECALL:
			return WC_KEY_KP_MEMRECALL;
		case SDLK_KP_MEMCLEAR:
			return WC_KEY_KP_MEMCLEAR;
		case SDLK_KP_MEMADD:
			return WC_KEY_KP_MEMADD;
		case SDLK_KP_MEMSUBTRACT:
			return WC_KEY_KP_MEMSUBTRACT;
		case SDLK_KP_MEMMULTIPLY:
			return WC_KEY_KP_MEMMULTIPLY;
		case SDLK_KP_MEMDIVIDE:
			return WC_KEY_KP_MEMDIVIDE;
		case SDLK_KP_PLUSMINUS:
			return WC_KEY_KP_PLUSMINUS;
		case SDLK_KP_CLEAR:
			return WC_KEY_KP_CLEAR;
		case SDLK_KP_CLEARENTRY:
			return WC_KEY_KP_CLEARENTRY;
		case SDLK_KP_BINARY:
			return WC_KEY_KP_BINARY;
		case SDLK_KP_OCTAL:
			return WC_KEY_KP_OCTAL;
		case SDLK_KP_DECIMAL:
			return WC_KEY_KP_DECIMAL;
		case SDLK_KP_HEXADECIMAL:
			return WC_KEY_KP_HEXADECIMAL;
		case SDLK_LCTRL:
			return WC_KEY_LCTRL;
		case SDLK_LSHIFT:
			return WC_KEY_LSHIFT;
		case SDLK_LALT:
			return WC_KEY_LALT;
		case SDLK_LGUI:
			return WC_KEY_LGUI;
		case SDLK_RCTRL:
			return WC_KEY_RCTRL;
		case SDLK_RSHIFT:
			return WC_KEY_RSHIFT;
		case SDLK_RALT:
			return WC_KEY_RALT;
		case SDLK_RGUI:
			return WC_KEY_RGUI;
		case SDLK_MODE:
			return WC_KEY_MODE;
		case SDLK_MEDIA_NEXT_TRACK:
			return WC_KEY_AUDIONEXT;
		case SDLK_MEDIA_PREVIOUS_TRACK:
			return WC_KEY_AUDIOPREV;
		case SDLK_MEDIA_STOP:
			return WC_KEY_AUDIOSTOP;
		case SDLK_MEDIA_PLAY:
			return WC_KEY_AUDIOPLAY;
		case SDLK_MEDIA_SELECT:
			return WC_KEY_MEDIASELECT;
		case SDLK_AC_SEARCH:
			return WC_KEY_AC_SEARCH;
		case SDLK_AC_HOME:
			return WC_KEY_AC_HOME;
		case SDLK_AC_BACK:
			return WC_KEY_AC_BACK;
		case SDLK_AC_FORWARD:
			return WC_KEY_AC_FORWARD;
		case SDLK_AC_STOP:
			return WC_KEY_AC_STOP;
		case SDLK_AC_REFRESH:
			return WC_KEY_AC_REFRESH;
		case SDLK_AC_BOOKMARKS:
			return WC_KEY_AC_BOOKMARKS;
		case SDLK_MEDIA_EJECT:
			return WC_KEY_EJECT;
		case SDLK_SLEEP:
			return WC_KEY_SLEEP;
		default:
			return WC_KEY_UNKNOWN;
	}
}

static war_app_t s_app;

void wc_app_init(const char* window_title, const AppCallbacks callbacks)
{
	s_app.callbacks = callbacks;

	if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init: %s\n", SDL_GetError());
		return;
	}

	Config config = {0};
	if (wc_config_load(&config, "settings.cfg") != 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load settings.cfg\n");
		return;
	}

	const u64 tick_frequency = SDL_GetPerformanceFrequency();
	s_app.time.tick_frequency = (double)tick_frequency;
	s_app.time.tick_inverse_frequency = 1.0f / (double)tick_frequency;
	s_app.time.tick_previous = SDL_GetPerformanceCounter() - (u64)(WC_FIXED_TIMESTEP * s_app.time.tick_frequency);
	s_app.time.accumulator = 0.0;

	const int resolution_x = wc_config_get_int(&config, "resolution_x", 1280);
	const int resolution_y = wc_config_get_int(&config, "resolution_y", 720);
	const bool fullscreen = wc_config_get_int(&config, "fullscreen", false);

	u32 window_flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
	window_flags |= SDL_WINDOW_VULKAN;
	if (fullscreen)
		window_flags |= SDL_WINDOW_FULLSCREEN;

	SDL_Window* sdl_window = SDL_CreateWindow(window_title, resolution_x, resolution_y, window_flags);
	if (!sdl_window)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow: %s\n", SDL_GetError());
		return;
	}

	s_app.window.handle = sdl_window;
	s_app.window.maximized = fullscreen;
	SDL_GetWindowPosition(sdl_window, &s_app.window.x, &s_app.window.y);
	SDL_GetWindowSize(sdl_window, &s_app.window.width, &s_app.window.height);

	s_app.running = true;
}

void wc_app_quit()
{
	if (s_app.callbacks.quit != NULL)
		s_app.callbacks.quit();

	SDL_DestroyWindow(s_app.window.handle);
	SDL_Quit();
}

bool wc_app_is_running()
{
	return s_app.running;
}

static void wc_handle_events()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_EVENT_QUIT:
				s_app.running = false;
			case SDL_EVENT_WINDOW_RESIZED:
				s_app.window.resized = true;
				s_app.window.width = event.window.data1;
				s_app.window.height = event.window.data2;
				break;

			case SDL_EVENT_WINDOW_MOVED:
				s_app.window.moved = true;
				s_app.window.x = event.window.data1;
				s_app.window.y = event.window.data2;
				break;

			case SDL_EVENT_WINDOW_MINIMIZED:
				s_app.window.minimized = true;
				break;

			case SDL_EVENT_WINDOW_MAXIMIZED:
				s_app.window.maximized = true;
				break;

			case SDL_EVENT_WINDOW_RESTORED:
				s_app.window.restored = true;
				break;

			case SDL_EVENT_WINDOW_MOUSE_ENTER:
				s_app.window.mouse_inside_window = true;
				break;

			case SDL_EVENT_WINDOW_MOUSE_LEAVE:
				s_app.window.mouse_inside_window = false;
				break;

			case SDL_EVENT_WINDOW_FOCUS_GAINED:
				s_app.window.has_keyboard_focus = true;
				break;

			case SDL_EVENT_WINDOW_FOCUS_LOST:
				s_app.window.has_keyboard_focus = false;
				break;

			case SDL_EVENT_KEY_DOWN:
			{
				if (event.key.repeat)
					continue;
				SDL_Keycode key = SDL_GetKeyFromScancode(event.key.scancode, event.key.mod, true);
				key = s_map_SDL_keys(key);
				assert(key < 512);
				s_app.keyboard.keys[key] = 1;
				s_app.keyboard.keys[WC_KEY_ANY] = 1;
				s_app.keyboard.keys_timestamp[key] = s_app.keyboard.keys_timestamp[WC_KEY_ANY] = s_app.time.seconds;
			}
			break;

			case SDL_EVENT_KEY_UP:
			{
				if (event.key.repeat)
					continue;
				const SDL_Keycode sdl_key = SDL_GetKeyFromScancode(event.key.scancode, event.key.mod, true);
				const int key = s_map_SDL_keys(sdl_key);
				assert(key >= 0 && key < 512);
				s_app.keyboard.keys[key] = 0;
			}
			break;

				// case SDL_EVENT_TEXT_INPUT:
				// {
				// 	wc_input_text_add_utf8(event.text.text);
				// 	s_app.ime_composition.clear();
				// 	s_app.ime_composition_cursor = 0;
				// 	s_app.ime_composition_selection_len = 0;
				// }	break;
				//
				// case SDL_EVENT_TEXT_EDITING:
				// {
				// 	s_app.ime_composition.clear();
				// 	const char* text = event.edit.text;
				// 	while (*text) s_app.ime_composition.add(*text++);
				// 	s_app.ime_composition.add(0);
				// 	s_app.ime_composition_cursor = event.edit.start;
				// 	s_app.ime_composition_selection_len = event.edit.length;
				// }	break;

			case SDL_EVENT_MOUSE_MOTION:
				s_app.mouse.x = event.motion.x;
				s_app.mouse.y = event.motion.y;
				s_app.mouse.delta_x = event.motion.xrel;
				s_app.mouse.delta_y = -event.motion.yrel;
				break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				switch (event.button.button)
				{
					case SDL_BUTTON_LEFT:
						s_app.mouse.button_left = 1;
						break;
					case SDL_BUTTON_RIGHT:
						s_app.mouse.button_right = 1;
						break;
					case SDL_BUTTON_MIDDLE:
						s_app.mouse.button_middle = 1;
						break;
					case SDL_BUTTON_X1:
						s_app.mouse.button_x1 = 1;
						break;
					case SDL_BUTTON_X2:
						s_app.mouse.button_x2 = 1;
						break;
					default:
						break;
				}
				s_app.mouse.x = event.button.x;
				s_app.mouse.y = event.button.y;
				if (event.button.clicks == 1)
				{
					s_app.mouse.double_click = false;
				}
				else if (event.button.clicks == 2)
				{
					s_app.mouse.double_click = true;
				}
				break;

			case SDL_EVENT_MOUSE_BUTTON_UP:
				switch (event.button.button)
				{
					case SDL_BUTTON_LEFT:
						s_app.mouse.button_left = 0;
						break;
					case SDL_BUTTON_RIGHT:
						s_app.mouse.button_right = 0;
						break;
					case SDL_BUTTON_MIDDLE:
						s_app.mouse.button_middle = 0;
						break;
					case SDL_BUTTON_X1:
						s_app.mouse.button_x1 = 0;
						break;
					case SDL_BUTTON_X2:
						s_app.mouse.button_x2 = 0;
						break;
					default:
						break;
				}
				s_app.mouse.x = event.button.x;
				s_app.mouse.y = event.button.y;
				if (event.button.clicks == 1)
				{
					s_app.mouse.double_click = false;
				}
				else if (event.button.clicks == 2)
				{
					s_app.mouse.double_click = true;
				}
				break;

			case SDL_EVENT_MOUSE_WHEEL:
				s_app.mouse.wheel = event.wheel.y;
				break;
			default:
				break;
		}
	}
}

void wc_app_update()
{
	s_app.time.tick = SDL_GetPerformanceCounter();
	const double delta_time = (double)(s_app.time.tick - s_app.time.tick_previous) * s_app.time.tick_inverse_frequency;
	s_app.time.tick_previous = s_app.time.tick;
	s_app.time.seconds = (double)s_app.time.tick * s_app.time.tick_inverse_frequency;

	s_app.time.accumulator += delta_time;
	// Clamp the accumulator to a maximum value to avoid "spiral of death"
	if (s_app.time.accumulator > WC_MAX_ACCUMULATOR)
	{
		s_app.time.accumulator = WC_MAX_ACCUMULATOR;
	}

	wc_handle_events();

	while (s_app.time.accumulator >= WC_FIXED_TIMESTEP)
	{
		// Update the game logic with a fixed, constant delta time.
		if (s_app.callbacks.update != NULL)
			s_app.callbacks.update(WC_FIXED_TIMESTEP);

		// Decrease the accumulator by the fixed step amount.
		s_app.time.accumulator -= WC_FIXED_TIMESTEP;
	}

	const double interpolant = s_app.time.accumulator / WC_FIXED_TIMESTEP;
	if (s_app.callbacks.render != NULL)
		s_app.callbacks.render(interpolant);
}

void* wc_app_get_window_handle()
{
	return s_app.window.handle;
}

void wc_app_get_window_size(int* width, int* height)
{
	assert(s_app.window.handle);
	*width = s_app.window.width;
	*height = s_app.window.height;
}
