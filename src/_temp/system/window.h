#pragma once

#include "std.h"

enum
{
	WB_DEFAULT_WINDOWED_WIDTH = 1280,
	WB_DEFAULT_WINDOWED_HEIGHT = 720,
};

typedef enum
{
	WB_WINDOW_MODE_WINDOWED,
	WB_WINDOW_MODE_FULLSCREEN,
	WB_WINDOW_MODE_BORDERLESS
} wb_window_mode;

typedef struct
{
	u32 window_width;
	u32 window_height;
	wb_window_mode window_mode;
	bool vsync;
} wb_client_config;

void wb_client_quit(void);
void wb_client_get_window_size(u32* width, u32* height);