#pragma once

#include <stdint.h>

typedef struct config_t config_t;

typedef struct camera_t
{
    int16_t x, y;
    uint16_t height;
} camera_t;

extern config_t* c_camera_zoom;
extern config_t* c_camera_speed;

