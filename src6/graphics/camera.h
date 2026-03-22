#pragma once

#include "std.h"

typedef struct {
    s16 x;
    s16 y;
    s32 zoom;
} cli_camera;

void cli_camera_translate(cli_camera* camera, s16 x, s16 y);