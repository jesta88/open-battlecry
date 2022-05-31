#pragma once

#include "../core/types.h"

enum
{
	WB_MAX_WINDOW_NAME_LENGTH = 128
};

typedef enum
{
	WB_WINDOW_WINDOWED,
	WB_WINDOW_FULLSCREEN,
	WB_WINDOW_BORDERLESS
} wb_window_mode;

typedef struct { uint32_t id; } wb_buffer;
typedef struct { uint32_t id; } wb_texture;
typedef struct { uint32_t id; } wb_shader;
typedef struct { uint32_t id; } wb_descriptor;
typedef struct { uint32_t id; } wb_pipeline;

typedef struct wb_graphics_state wb_graphics_state;

typedef struct
{
	char window_name[WB_MAX_WINDOW_NAME_LENGTH];
	int window_width;
	int window_height;
	wb_window_mode window_mode;

	bool enable_validation;
	bool vsync;
} wb_graphics_desc;

typedef struct
{
	int size;
	void* data;
} wb_buffer_desc;

typedef struct
{
	int width;
	int height;
} wb_texture_desc;

void wb_graphics_init(const wb_graphics_desc* graphics_desc);
void wb_graphics_quit(void);

wb_buffer wb_create_buffer(const wb_buffer_desc* buffer_desc);
wb_texture wb_create_texture(const wb_texture_desc* texture_desc);

wb_graphics_state* wb_graphics_get_state(void);