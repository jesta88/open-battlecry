#pragma once

#include "../base/types.h"

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

void graphics_init(void);
void graphics_quit(void);