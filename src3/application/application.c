#include <warkit/application.h>

struct wk_app
{
    int argc;
    char** argv;
};

int wk_main(int argc, char* argv[])
{
    wk_engine_api engine_api = { 0 };

    const wk_game_desc game_desc = wk_game_main(argc, argv);

    // Engine
    wk_set_win32_hinstance(g_win32_hinstance);

    const wk_engine_desc engine_desc = {
        .target_tickrate = WK_TICKRATE_240
    };

    if (wk_init_engine(&engine_desc) != 0)
        return 1;

    // Window
    const wk_window_desc window_desc = {
        .title = game_desc.title,
        .width = 1280,
        .height = 720,
        .fullscreen = false,
    };

    if (wk_open_window(&window_desc) != 0)
        return 1;

    wk_show_window();

    // Main loop
    bool quit = false;
    wk_event window_event;
    float lag = 0.0f;
    uint64 elapsed = 0;
    uint64 current_tick = 0;
    uint64 previous_tick = wk_get_ticks_ns();
    while (!quit)
    {
        current_tick = wk_get_ticks_ns();
        elapsed = current_tick - previous_tick;
        previous_tick = current_tick;
        lag += (float)elapsed / 1000.0f;

        quit = !wk_poll_events(&window_event);
    }

    wk_quit_engine();

    return 0;
}
