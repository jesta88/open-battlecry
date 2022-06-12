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
} WbWindowMode;