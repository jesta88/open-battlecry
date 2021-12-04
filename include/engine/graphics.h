#ifndef SERVER
#pragma once

#include "types.h"

typedef struct { uint8_t index, generation; } ws_shader;
typedef struct { uint8_t index, generation; } ws_pipeline;
typedef struct { uint8_t index, generation; } ws_descriptor_set;
typedef struct { uint16_t index, generation; } ws_buffer;
typedef struct { uint16_t index, generation; } ws_texture;

typedef enum {
    WS_QUEUE_TYPE_GRAPHICS,
    WS_QUEUE_TYPE_TRANSFER,
    WS_QUEUE_TYPE_COMPUTE
} ws_queue_type;

typedef enum {
    WS_SHADER_STAGE_VERTEX,
    WS_SHADER_STAGE_FRAGMENT,
    WS_SHADER_STAGE_COMPUTE
} ws_shader_stage;

typedef struct {
    uint8_t* code;
    uint32_t code_size;
    uint32_t shader_stage;
} ws_shader_desc;

typedef enum {
    WS_RESOURCE_STATE_INITIAL,
    WS_RESOURCE_STATE_ALLOC,
    WS_RESOURCE_STATE_VALID,
    WS_RESOURCE_STATE_FAILED,
    WS_RESOURCE_STATE_INVALID,
    _WS_RESOURCE_STATE_FORCE_U32 = 0x7FFFFFFF
} ws_resource_state;

typedef enum {
    WS_RESOURCE_USAGE_IMMUTABLE,
    WS_RESOURCE_USAGE_DYNAMIC,
    WS_RESOURCE_USAGE_STREAM,
    _WS_RESOURCE_USAGE_COUNT,
    _WS_RESOURCE_USAGE_FORCE_U32 = 0x7FFFFFFF
} ws_resource_usage;

typedef enum {
    WS_BUFFER_TYPE_VERTEX,
    WS_BUFFER_TYPE_INDEX,
    WS_BUFFER_TYPE_INDIRECT,
    _WS_BUFFER_TYPE_COUNT,
    _WS_BUFFER_TYPE_FORCE_U32 = 0x7FFFFFFF
} ws_buffer_type;

typedef struct {
    uint16_t type;
    uint16_t usage;
    uint32_t size;
    void* data;
} ws_buffer_desc;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint16_t format;
    uint16_t usage;
    uint32_t data_size;
    void *data;
} ws_texture_desc;

void ws_init_graphics(void *window_handle);
void ws_quit_graphics(void);

ws_buffer ws_create_buffer(const ws_buffer_desc* desc);
ws_shader ws_create_shader(const ws_shader_desc* desc);
ws_texture ws_create_texture(const ws_texture_desc* desc);

#endif