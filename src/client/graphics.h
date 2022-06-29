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
	float window_width;
	float window_height;
	float camera_x;
	float camera_y;
} wb_push_constants;

typedef struct
{
    float x, y;
    float width, height;
    u32 texture_index;
    u32 color;
	u32 _pad0;
	u32 _pad1;
} wb_sprite;

void wb_graphics_init(const WbGraphicsDesc* graphics_desc);
void wb_graphics_quit(void);
void wb_graphics_draw(void);