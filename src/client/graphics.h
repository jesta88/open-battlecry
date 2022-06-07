#pragma once

#include "../common/types.h"

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

typedef struct
{
	int width;
	int height;
} wb_render_target_desc;

void wb_graphics_init(const wb_graphics_desc* graphics_desc);
void wb_graphics_quit(void);
void wb_graphics_clear(void);
void wb_graphics_present(void);

void wb_graphics_set_clear_color(u8 r, u8 g, u8 b, u8 a);
void wb_graphics_set_render_target(void* render_target);
void wb_graphics_set_null_render_target(void);

u16 wb_graphics_create_buffer(const wb_buffer_desc* buffer_desc);
u16 wb_graphics_create_texture(const wb_texture_desc* texture_desc);
u16 wb_graphics_create_render_target(const wb_render_target_desc* render_target_desc);
