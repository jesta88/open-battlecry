#pragma once

#include "../common/types.h"

typedef struct
{
	int window_width;
	int window_height;
#ifdef _WIN32
	void* hwnd;
	void* hinstance;
#endif
	bool enable_validation;
	bool vsync;
} WbGraphicsDesc;

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

void wbInitGraphics(const WbGraphicsDesc* graphics_desc);
void wbFreeGraphics(void);

void wbBeginFrame(void);
void wbEndFrame(void);

void wb_graphics_set_clear_color(u8 r, u8 g, u8 b, u8 a);
void wb_graphics_set_render_target(void* render_target);
void wb_graphics_set_null_render_target(void);

u16 wb_graphics_create_buffer(const wb_buffer_desc* buffer_desc);
u16 wb_graphics_create_texture(const wb_texture_desc* texture_desc);
u16 wb_graphics_create_render_target(const wb_render_target_desc* render_target_desc);
