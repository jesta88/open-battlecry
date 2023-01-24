#include <ktxvulkan.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

static HWND      s_hwnd;
static LPCSTR    k_title = "Texture Viewer";

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    (void)hPrevInstance;
    if (!*lpCmdLine)
    {
        MessageBoxA(NULL, "Usage: texture_viewer <filename>", k_title, MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    ktxTexture2* texture;
    KTX_error_code result = ktxTexture2_CreateFromNamedFile(lpCmdLine, KTX_TEXTURE_CREATE_NO_FLAGS, &texture);
    if (result != KTX_SUCCESS)
    {
        char buffer[2048] = {0};
        sprintf(buffer, "Failed to open texture file\n\nFilename = %ls\n", lpCmdLine);
        MessageBoxA(NULL, buffer, k_title, MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    ktx_uint32_t texture_width = texture->baseWidth;
    ktx_uint32_t texture_height = texture->baseHeight;

    if (FAILED(InitWindow(hInstance, nCmdShow, texture_width, texture_height)))
        return 0;

    SetWindowTextA(s_hwnd, lpCmdLine);

    // TODO: Create Vulkan device

    // Main message loop
    MSG msg = {0};
    while (WM_QUIT != msg.message)
    {
        if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        else
        {
            // TODO: Render
        }
    }

    // TODO: Destroy Vulkan device

    return msg.wParam;
}

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow, LONG width, LONG height)
{
    // Register class
    WNDCLASSEXA wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEXA);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL; //LoadIconA(hInstance, (LPCSTR)IDI_MAIN_ICON);
    wcex.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = "TextureViewerClass";
    wcex.hIconSm = NULL; //LoadIconA(wcex.hInstance, (LPCSTR)IDI_MAIN_ICON);
    if (!RegisterClassExA(&wcex))
        return E_FAIL;

    // Create window
    RECT rc = { 0, 0, 1280, 720 };

    int cxborder = GetSystemMetrics(SM_CXBORDER);
    int cxedge = GetSystemMetrics(SM_CXEDGE);
    int screenX = GetSystemMetrics(SM_CXSCREEN) - max(cxborder, cxedge);
    if (rc.right < width)
        rc.right = width;
    if (rc.right > screenX)
        rc.right = screenX;

    int cyborder = GetSystemMetrics(SM_CYBORDER);
    int cyedge = GetSystemMetrics(SM_CYEDGE);
    int screenY = GetSystemMetrics(SM_CYSCREEN) - max(cyborder, cyedge);
    if (rc.bottom < height)
        rc.bottom = height;
    if (rc.bottom > screenY)
        rc.bottom = screenY;

    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    s_hwnd = CreateWindowA("TextureViewerClass", k_title, WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL);
    if (!s_hwnd)
        return E_FAIL;

    ShowWindow(s_hwnd, nCmdShow);

    return S_OK;
}