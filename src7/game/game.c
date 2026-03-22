#include "../_temp/engine.h"

static bool init(void)
{
    return true;
}

engine_desc engine_run(int argc, const char* argv[])
{
    (void)argc;
    (void)argv;

    return (engine_desc) {
        .name = "Open Battlecry",
        .window_width = 1280,
        .window_height = 720,
        .init = init
    };
}