#pragma once

#include <stdbool.h>
#include <stdint.h>

enum
{
    ANI_STAND     = 0,
    ANI_WALK      = 1,
    ANI_FIGHT     = 2,
    ANI_DIE       = 3,
    ANI_AMBIENT   = 4,
    ANI_SPECIAL   = 5,
    ANI_CONVERT   = 6,
    ANI_SPELL     = 7,
    ANI_INTERFACE = 8,
    ANI_TYPE_COUNT = 9,

    ANI_DIR_COUNT = 8,
    ANI_DIR_SOUTH = 4,
};

typedef struct
{
    bool used;
    uint8_t num_frames;
    int16_t origin_x;
    int16_t origin_y;
    int16_t width;
    int16_t height;
} ani_type;

typedef struct
{
    ani_type types[ANI_TYPE_COUNT];
} ani_info;

// Parse an ANI resource blob (234 bytes new format, 180 bytes old format).
bool ani_parse(const uint8_t* data, uint32_t size, ani_info* out);

// --- Animation player (runtime state) ---

typedef struct
{
    uint8_t anim_type;
    uint8_t direction;
    uint8_t frame;
    uint8_t num_frames;
    float timer;
    float frame_duration;
} anim_player;

// Set the animation type and direction. Resets frame to 0.
void anim_player_set(anim_player* p, const ani_info* info, uint8_t anim_type, uint8_t direction);

// Advance time. Returns true when a full cycle completes.
bool anim_player_update(anim_player* p, float dt);

// Compute UV rectangle for the current frame/direction within a sprite sheet.
void anim_player_uv(const anim_player* p, const ani_type* type,
                     uint32_t tex_width, uint32_t tex_height,
                     float uv_offset[2], float uv_scale[2]);
