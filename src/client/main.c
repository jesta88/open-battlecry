#include "../common/input.h"
#include "../common/log.h"
#include "graphics.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>

enum
{
	DEFAULT_WINDOW_WIDTH = 1280,
	DEFAULT_WINDOW_HEIGHT = 720,

	MAX_WINDOW_EVENTS = 8,
	MAX_KEYBOARD_EVENTS = 64,
	MAX_MOUSE_EVENTS = 256,
};

static const char* k_window_class = "open_wbc";
static const char* k_window_title = "Open WBC";

static bool s_quit;
static bool s_fullscreen;
static bool s_maximized;
static RECT s_windowed_rect;
static int s_window_x;
static int s_window_y;
static int s_window_width;
static int s_window_height;

static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static void adjustWindow(HWND hwnd);

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			wbLogInfo("Args[%d]: %s", i, argv[i]);
		}
	}

	HINSTANCE instance = GetModuleHandle(NULL);
	WNDCLASSEX window_class = {
			.cbSize = sizeof(window_class),
			.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
			.lpfnWndProc = windowProc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = instance,
			.hIcon = LoadIcon(NULL, IDI_APPLICATION),
			.hCursor = LoadCursor(NULL, IDC_ARROW),
			.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
			.lpszMenuName = NULL,
			.lpszClassName = k_window_class,
			.hIconSm = 0};
	RegisterClassEx(&window_class);

	int window_style = WS_OVERLAPPEDWINDOW;
	RECT window_rect;
	window_rect.left = 0;
	window_rect.top = 0;
	window_rect.right = DEFAULT_WINDOW_WIDTH;
	window_rect.bottom = DEFAULT_WINDOW_HEIGHT;
	AdjustWindowRect(&window_rect, window_style, FALSE);

	HWND hwnd = CreateWindowEx(
			0,
			k_window_class,
			k_window_title,
			window_style,
			CW_USEDEFAULT, CW_USEDEFAULT,
			window_rect.right - window_rect.left,
			window_rect.bottom - window_rect.top,
			0,
			NULL,
			instance,
			NULL);
	assert(hwnd != NULL);

	wbInitGraphics(&(const WbGraphicsDesc) {
		.enable_validation = true,
		.window_handle = hwnd,
		.window_width = 1280,
		.window_height = 720,
		.vsync = false
	});

	ShowWindow(hwnd, SW_SHOW);

	while (!s_quit)
	{
		MSG msg = {0};
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				s_quit = true;
			}
		}
	}

	return 0;
}

static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
			adjustWindow(hwnd);
			break;
		}
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;

			// Prevent window from collapsing
			if (!s_fullscreen)
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
			HDC hdc = (HDC)wParam;
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

static void adjustWindow(HWND hwnd)
{
	if (s_fullscreen)
	{
		// Save the old window rect so we can restore it when exiting fullscreen mode.
		GetWindowRect(hwnd, &s_windowed_rect);

		// Make the window borderless so that the client area can fill the screen.
		SetWindowLong(hwnd, GWL_STYLE, WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);

		// Get the settings of the durrent display index. We want the app to go into
		// fullscreen mode on the display that supports Independent Flip.
		HMONITOR currentMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO info;
		info.cbSize = sizeof(MONITORINFOEX);
		GetMonitorInfo(currentMonitor, &info);

		s_window_x = info.rcMonitor.left;
		s_window_y = info.rcMonitor.top;

		int window_width = info.rcMonitor.right - info.rcMonitor.left;
		int window_height = info.rcMonitor.bottom - info.rcMonitor.top;
		SetWindowPos(hwnd, HWND_NOTOPMOST, info.rcMonitor.left, info.rcMonitor.top, window_width, window_height, SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(hwnd, SW_MAXIMIZE);

		s_window_width = window_width;
		s_window_height = window_height;
	}
	else
	{
		DWORD windowStyle = WS_OVERLAPPEDWINDOW;
		SetWindowLong(hwnd, GWL_STYLE, windowStyle);

		SetWindowPos(
				hwnd, HWND_NOTOPMOST, s_windowed_rect.left, s_windowed_rect.top, s_windowed_rect.right - s_windowed_rect.left,
				s_windowed_rect.bottom - s_windowed_rect.top, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

		if (s_maximized)
		{
			ShowWindow(hwnd, SW_MAXIMIZE);
		}
		else
		{
			ShowWindow(hwnd, SW_NORMAL);
		}
	}
}