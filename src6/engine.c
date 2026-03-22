#include "engine.h"
#include "engine_private.h"
#include "timer.h"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

engine g_engine;

void engine_run(const engine_desc* desc)
{
    io_init();
    timer_init();

    uint64_t previous_ticks = timer_ticks();
    bool quit = false;
    while (!quit)
    {
        uint64_t  current_ticks = timer_ticks();
        uint64_t  elapsed_ticks = current_ticks - previous_ticks;
        previous_ticks = current_ticks;
        float delta_time = ((float)elapsed_ticks / 1000.0f);


    }
}
