#pragma once

#include "ani.h"
#include "xcr.h"
#include <stdbool.h>
#include <stdint.h>

enum
{
    UNIT_STATE_IDLE     = 0,
    UNIT_STATE_WALKING  = 1,
    UNIT_STATE_FIGHTING = 2,
    UNIT_STATE_DYING    = 3,

    MAX_UNITS = 128,
};

// Shared data for one unit class
typedef struct
{
    uint32_t tex[ANI_TYPE_COUNT];
    uint32_t sheet_w[ANI_TYPE_COUNT];
    uint32_t sheet_h[ANI_TYPE_COUNT];
    ani_info ani;
    char base_code[8];

    // Combat stats
    int16_t max_health;
    int16_t attack_damage;
    int16_t armor;
    int16_t combat_skill;
    float attack_range;
    float aggro_range;
    float attack_cooldown;
} unit_type;

// Per-instance unit data
typedef struct
{
    float x, y;
    float target_x, target_y;
    float speed;

    uint8_t state;
    uint8_t team;        // 0 = player, 1 = enemy
    bool selected;

    int16_t health;
    float attack_timer;
    uint32_t target_unit; // index into unit_array, UINT32_MAX if none

    anim_player anim;
    const unit_type* type;
} unit;

typedef struct
{
    unit units[MAX_UNITS];
    uint32_t count;
} unit_array;

bool unit_type_load(unit_type* ut, const xcr_archive* archive, const char* base_code);

void unit_type_set_default_stats(unit_type* ut, int16_t health, int16_t damage,
                                  int16_t armor, int16_t combat,
                                  float attack_range, float aggro_range,
                                  float attack_cooldown);

uint32_t unit_spawn(unit_array* arr, const unit_type* type, float x, float y, uint8_t team);

void units_update(unit_array* arr, float dt);

// white_tex: 1x1 white texture index for health bar rendering
void units_draw(const unit_array* arr, uint32_t white_tex);
