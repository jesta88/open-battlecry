#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

typedef struct timer
{
    uint64_t frequency;
} timer;

typedef struct engine
{
    char base_path[MAX_PATH];
    char user_path[MAX_PATH];

    struct timer timer;

    void (*update)(float delta_time);
    void (*draw)(void);
} engine;

engine g_engine;

void engine_run(const engine_desc* desc);

void io_init(void);
void timer_init(void);
bool events_poll(void);