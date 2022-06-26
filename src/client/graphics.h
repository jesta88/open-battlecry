#pragma once

#include "../common/types.h"

typedef struct
{
	int window_width;
	int window_height;

	u32 max_sampled_images;
	u32 max_storage_buffers;

#ifdef _WIN32
	void* hwnd;
	void* hinstance;
#endif
	bool vsync;
} WbGraphicsDesc;

typedef struct
{
	int size;
	void* data;
} WbBufferDesc;

typedef struct
{
	int width;
	int height;
} WbTextureDesc;

typedef struct
{
	int width;
	int height;
} WbRenderTargetDesc;

typedef struct
{
	u32 color;
	u32 texture_index;
} WbSpriteDesc;

void wbInitGraphics(const WbGraphicsDesc* graphics_desc);
void wbFreeGraphics(void);

void wbDraw(void);

void wb_graphics_set_clear_color(u8 r, u8 g, u8 b, u8 a);
void wb_graphics_set_render_target(void* render_target);
void wb_graphics_set_null_render_target(void);

u16 wb_graphics_create_buffer(const WbBufferDesc* buffer_desc);
u16 wb_graphics_create_texture(const WbTextureDesc* texture_desc);
u16 wb_graphics_create_render_target(const WbRenderTargetDesc* render_target_desc);
