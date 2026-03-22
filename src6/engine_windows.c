#ifdef _WIN32
#include "engine.h"
#include "engine_private.h"
#include "log.h"
#include <assert.h>
#include <stdlib.h>
// TODO: Use minimal windows headers
#include <windows.h>
#include <shellapi.h>

const char* k_window_class = "application";

static LRESULT __stdcall window_procedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);
static const char** parse_command_line(int* argc);

bool engine_init(void)
{

}

void engine_quit(void)
{

}

void app_open_window(void)
{
    WNDCLASSEXA window_class = {.cbSize = sizeof(window_class),
                                .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
                                .lpfnWndProc = window_procedure,
                                .cbClsExtra = 0,
                                .cbWndExtra = 0,
                                .hInstance = client.window.hinstance,
                                .hIcon = LoadIconA(NULL, (LPCSTR)IDI_APPLICATION),
                                .hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW),
                                .hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
                                .lpszMenuName = NULL,
                                .lpszClassName = k_window_class,
                                .hIconSm = 0};
    RegisterClassExA(&window_class);

    int window_style = WS_OVERLAPPEDWINDOW;
    RECT window_rect = {
        .left = 0,
        .top = 0,
        .right = client.window.width,
        .bottom = client.window.height,
    };
    AdjustWindowRect(&window_rect, window_style, FALSE);

    client.window.hwnd = CreateWindowExA(0, k_window_class, client.window.title, window_style, CW_USEDEFAULT,
                                         CW_USEDEFAULT, window_rect.right - window_rect.left,
                                         window_rect.bottom - window_rect.top, 0, NULL, client.window.hinstance, NULL);
}

bool app_handle_events(void)
{
    bool quit = false;
    MSG msg = {0};
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);

        if (msg.message == WM_QUIT)
        {
            quit = true;
        }
    }
    return quit;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int show_cmd)
{
    int argc;
    const char** argv = parse_command_line(&argc);

    engine_desc desc = engine_main(argc, argv);
    engine_run(&desc);
}

static LRESULT CALLBACK window_procedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message)
    {
        case WM_NCPAINT:
        case WM_WINDOWPOSCHANGED:
        case WM_STYLECHANGED: {
            return DefWindowProcA(hwnd, message, w_param, l_param);
        }
        case WM_DISPLAYCHANGE: {
            adjust_window();
            break;
        }
        case WM_GETMINMAXINFO: {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)l_param;

            // Prevent window from collapsing
            if (!client.window.fullscreen)
            {
                LONG zoom_offset = 128;
                lpMMI->ptMinTrackSize.x = zoom_offset;
                lpMMI->ptMinTrackSize.y = zoom_offset;
            }
            break;
        }
        case WM_ERASEBKGND: {
            // Make sure to keep consistent background color when resizing.
            HDC hdc = (HDC)w_param;
            RECT rc;
            HBRUSH hbrWhite = CreateSolidBrush(0x00000000);
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, hbrWhite);
            break;
        }
        case WM_CLOSE: {
            PostQuitMessage(0);
            break;
        }
        case WM_DESTROY:
            break;
        default:
            return DefWindowProcA(hwnd, message, w_param, l_param);
    }
    return 0;
}

static const char** parse_command_line(int* argc)
{
    int arg_count = 0;
    wchar_t* command_line = GetCommandLineW();
    wchar_t** argv_wide = CommandLineToArgvW(command_line, &arg_count);
    assert(argv_wide);

    uint64_t size = wcslen(command_line) * 4;
    char** argv = calloc(((uint64_t)arg_count + 1) * sizeof(char*) + size);
    assert(argv);
    char* args = argv[arg_count + 1];
    int n;
    for (int i = 0; i < arg_count; ++i)
    {
        n = WideCharToMultiByte(CP_UTF8, 0, argv_wide[i], -1, args, (int)size, NULL, NULL);
        if (n == 0)
        {
            log_error("Could not convert arguments to utf8.");
            break;
        }
        argv[i] = args;
        size -= (uint64_t)n;
        args += n;
    }
    LocalFree(argv_wide);
    *argc = arg_count;
    return (const char**)argv;
}

#endif // _WIN32