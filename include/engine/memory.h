#pragma once

#include "types.h"

typedef enum {
    WS_ALLOC_LIFETIME_FRAME,
    WS_ALLOC_LIFETIME_STATE,
    WS_ALLOC_LIFETIME_APP,
    WS_ALLOC_LIFETIME_CUSTOM
} ws_alloc_lifetime;

typedef struct {
    void* start;
    void* end;
    void* free;
} ws_stack_allocator;

ws_stack_allocator* ws_create_stack_allocator(uint32_t size);
void ws_destroy_stack_allocator(ws_stack_allocator* allocator);
void* ws_stack_alloc(ws_stack_allocator* allocator, uint32_t size);
void ws_stack_free(ws_stack_allocator* allocator);
void ws_stack_reset(ws_stack_allocator* allocator);

void* ws_frame_alloc(uint32_t size);
void* ws_scene_alloc(uint32_t size);