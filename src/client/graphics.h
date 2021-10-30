#pragma once

#include "../engine/types.h"

typedef struct config_t config_t;

typedef enum
{
    TEXTURE_USAGE_STATIC,
    TEXTURE_USAGE_DYNAMIC,
    TEXTURE_USAGE_RENDER_TARGET
} texture_usage;

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint16_t format;
    uint16_t usage;
    uint32_t data_size;
    void* data;
} texture_create_info;

extern config_t* c_vsync;

void graphics_init(void* window_handle);
void graphics_quit(void);