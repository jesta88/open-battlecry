#include "client.h"
#include "../common/log.h"
#include "graphics.h"
#include <assert.h>
#include <stdlib.h>

#ifdef _WIN64
#include <windows.h>
#include <shellapi.h>
#else
#include <SDL2/SDL.h>
#endif

typedef struct
{
	const char* title;
	int x;
	int y;
	u32 width;
	u32 height;
	u32 windowed_width;
	u32 windowed_height;
	bool fullscreen;
	bool maximized;
	bool needs_resize;

#ifdef _WIN64
	HWND hwnd;
#else
	SDL_Window* sdl_window;
#endif
} wb_window;

typedef struct
{
	int argc;
	char** argv;
	bool initialized;
	bool quit_requested;
	wb_client_config config;
	wb_window window;
} wb_client;

static wb_client client;

static const char* k_window_title = "Open WBC";
static const char* k_window_class = "open_wbc";

static void create_window(void);
static void show_window(void);
static void get_fullscreen_resolution(u32* width, u32* height);
static void adjust_window(void);
static void process_events(void);

#ifdef _WIN64
static LRESULT CALLBACK window_procedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
#else
int main(int argc, char* argv[])
#endif
{
	// Command line
#ifdef _WIN64
	LPWSTR* argv_w = CommandLineToArgvW(GetCommandLineW(), &client.argc);
#else
	client.argc = argc;
	client.argv = argv;
#endif

	// Config
	// TODO: Load .ini for client config
	client.config.vsync = false;
	client.config.window_mode = WB_WINDOW_MODE_WINDOWED;
	client.config.window_width = WB_DEFAULT_WINDOWED_WIDTH;
	client.config.window_height = WB_DEFAULT_WINDOWED_HEIGHT;

	client.window.title = k_window_title;
	client.window.windowed_width = WB_DEFAULT_WINDOWED_WIDTH;
	client.window.windowed_height = WB_DEFAULT_WINDOWED_HEIGHT;

	if (client.config.window_mode == WB_WINDOW_MODE_WINDOWED)
	{
		client.window.width = client.config.window_width;
		client.window.height = client.config.window_height;
		client.window.fullscreen = false;
	}
	else
	{
		u32 fullscreen_width = client.config.window_width;
		u32 fullscreen_height = client.config.window_height;
		if (fullscreen_width == 0 || fullscreen_height == 0)
		{
			get_fullscreen_resolution(&fullscreen_width, &fullscreen_height);
		}

		client.window.width = fullscreen_width;
		client.window.height = fullscreen_height;
		client.window.fullscreen = true;
	}

	create_window();

	const WbGraphicsDesc graphics_desc = {
		.window_width = client.window.width,
		.window_height = client.window.height,
		.vsync = false,
#ifdef _WIN64
		.hwnd = client.window.hwnd,
		.hinstance = hInstance,
#else

#endif
	};
	wb_graphics_init(&graphics_desc);

	show_window();

	while (!client.quit_requested)
	{
		process_events();

		if (client.quit_requested)
		{
			break;
		}

		wb_graphics_draw();
	}

	wb_graphics_quit();

#ifdef _WIN64
	LocalFree(argv_w);
#endif
	return EXIT_SUCCESS;
}

void wb_client_quit(void)
{
#ifdef WIN32
	PostQuitMessage(EXIT_SUCCESS);
#else
	SDL_Quit();
#endif
	client.initialized = false;
}

void wb_client_get_window_size(u32* width, u32* height)
{
	*width = client.window.width;
	*height = client.window.height;
}

static void create_window(void)
{
#ifdef _WIN64
	HINSTANCE hinstance = GetModuleHandle(NULL);
	WNDCLASSEX window_class = {
			.cbSize = sizeof(window_class),
			.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
			.lpfnWndProc = window_procedure,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = hinstance,
			.hIcon = LoadIcon(NULL, IDI_APPLICATION),
			.hCursor = LoadCursor(NULL, IDC_ARROW),
			.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH),
			.lpszMenuName = NULL,
			.lpszClassName = k_window_class,
			.hIconSm = 0};
	RegisterClassEx(&window_class);

	int window_style = WS_OVERLAPPEDWINDOW;
	RECT window_rect = {
		.left = 0,
		.top = 0,
		.right = client.window.width,
		.bottom = client.window.height,
	};
	AdjustWindowRect(&window_rect, window_style, FALSE);

	client.window.hwnd = CreateWindowEx(
			0,
			k_window_class,
			client.window.title,
			window_style,
			CW_USEDEFAULT, CW_USEDEFAULT,
			window_rect.right - window_rect.left,
			window_rect.bottom - window_rect.top,
			0,
			NULL,
			hinstance,
			NULL);
	assert(client.window.hwnd != NULL);
#else
	client.window.sdl_window = SDL_CreateWindow();
	assert(client.window.sdl_window);
#endif
}

static void get_fullscreen_resolution(u32* width, u32* height)
{
#ifdef _WIN64
	HMONITOR monitor = MonitorFromWindow(client.window.hwnd, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO info = {
		.cbSize = sizeof(MONITORINFOEX),
	};
	GetMonitorInfo(monitor, &info);

	*width = info.rcMonitor.right - info.rcMonitor.left;
	*height = info.rcMonitor.bottom - info.rcMonitor.top;
#endif
}

static void show_window(void)
{
#ifdef _WIN64
	ShowWindow(client.window.hwnd, SW_SHOW);
#else

#endif
}

static void process_events()
{
	MSG msg = {0};
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
		{
			client.quit_requested = true;
		}
	}
}

static void adjust_window()
{
	if (client.window.fullscreen)
	{
#ifdef _WIN64
		// Save the old window rect so we can restore it when exiting fullscreen mode.
		RECT windowed_rect;
		GetWindowRect(client.window.hwnd, &windowed_rect);
		client.window.windowed_width = windowed_rect.right - windowed_rect.left;
		client.window.windowed_height = windowed_rect.bottom - windowed_rect.top;

		// Make the window borderless so that the client area can fill the screen.
		SetWindowLong(client.window.hwnd, GWL_STYLE, WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);

		// Get the settings of the durrent display index. We want the app to go into
		// fullscreen mode on the display that supports Independent Flip.
		HMONITOR currentMonitor = MonitorFromWindow(client.window.hwnd, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO info = {
			.cbSize = sizeof(MONITORINFOEX),
		};
		GetMonitorInfo(currentMonitor, &info);

		client.window.x = info.rcMonitor.left;
		client.window.y = info.rcMonitor.top;

		int window_width = info.rcMonitor.right - info.rcMonitor.left;
		int window_height = info.rcMonitor.bottom - info.rcMonitor.top;
		SetWindowPos(client.window.hwnd, HWND_NOTOPMOST, info.rcMonitor.left, info.rcMonitor.top, window_width, window_height, SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(client.window.hwnd, SW_MAXIMIZE);

		client.window.width = window_width;
		client.window.height = window_height;
#else

#endif
	}
	else
	{
#ifdef _WIN64
		DWORD windowStyle = WS_OVERLAPPEDWINDOW;
		SetWindowLong(client.window.hwnd, GWL_STYLE, windowStyle);

		SetWindowPos(
				client.window.hwnd, HWND_NOTOPMOST, client.window.x, client.window.y, client.window.width, client.window.height,
				SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

		if (client.window.maximized)
		{
			ShowWindow(client.window.hwnd, SW_MAXIMIZE);
		}
		else
		{
			ShowWindow(client.window.hwnd, SW_NORMAL);
		}
#else

#endif
	}
}

#ifdef _WIN64
static LRESULT CALLBACK window_procedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_NCPAINT:
		case WM_WINDOWPOSCHANGED:
		case WM_STYLECHANGED:
		{
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
		case WM_DISPLAYCHANGE:
		{
			adjust_window();
			break;
		}
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;

			// Prevent window from collapsing
			if (!client.window.fullscreen)
			{
				LONG zoom_offset = 128;
				lpMMI->ptMinTrackSize.x = zoom_offset;
				lpMMI->ptMinTrackSize.y = zoom_offset;
			}
			break;
		}
		case WM_ERASEBKGND:
		{
			// Make sure to keep consistent background color when resizing.
			HDC hdc = (HDC) wParam;
			RECT rc;
			HBRUSH hbrWhite = CreateSolidBrush(0x00000000);
			GetClientRect(hwnd, &rc);
			FillRect(hdc, &rc, hbrWhite);
			break;
		}
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			break;
		}
		case WM_DESTROY:
			break;
		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}
#endif