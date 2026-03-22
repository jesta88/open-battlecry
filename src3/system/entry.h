#pragma once

#include <engine/application.h>

#if WK_PLATFORM_WINDOWS
#include <stdlib.h>

typedef struct HINSTANCE__* HINSTANCE;
typedef char* LPSTR;

__declspec(dllimport) __declspec(noreturn) void __stdcall ExitProcess(unsigned int uExitCode);

static HINSTANCE g_win32_hinstance;

__declspec(dllexport) int NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;
    g_win32_hinstance = hInstance;
    int result = wk_main(__argc, __argv);
    ExitProcess(result);
}
#else
int __cdecl main(int argc, char* argv[])
{
    return wk_main(argc, argv);
}
#endif