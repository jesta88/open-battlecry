#pragma once

#include "std.h"

typedef struct engine_allocator
{
    void* (*allocate)(uint64_t size);
    void* (*reallocate)(void* memory, uint64_t size);
    void (*deallocate)(void* memory);
} engine_allocator;

typedef struct engine_desc
{
    const char* name;
    uint32_t window_width;
    uint32_t window_height;
    uint32_t monitor_index;
    int32_t window_x;
    int32_t window_y;
    bool fullscreen;
    bool borderless;

    engine_allocator allocator;

    bool (*init)(void);
    void (*quit)(void);

    bool (*load_resources)(void);
    void (*unload_resources)(void);

    void (*update)(float delta_time);
    void (*draw)(void);
} engine_desc;

// Entry point of the application. Define this function in your game/application.
ENGINE_API engine_desc engine_main(int argc, const char* argv[]);