#pragma once

#include <stdint.h>

typedef struct scene_t scene_t;

typedef struct unit_t
{
    uint32_t id;
} unit_t;

void unit_add(scene_t* scene, const unit_t* unit, int32_t x, int32_t y);
