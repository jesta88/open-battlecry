#ifndef SERVER
#pragma once

#include "types.h"

typedef struct {
    int buffer_pool_size;
    int texture_pool_size;
    int shader_pool_size;
    int pipeline_pool_size;
    int pass_pool_size;

    void (*render)(void);
} ws_renderer_desc;

void renderer_init(void* window_handle);
void renderer_quit(void);
void renderer_draw(void);
void renderer_present(void);
#endif