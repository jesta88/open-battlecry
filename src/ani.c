#include "ani.h"

#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
// On-disk ANI format (from WBC3 original: extern.h xANIMATION_TYPE)
// ---------------------------------------------------------------------------

#pragma pack(push, 1)

typedef struct
{
    char used;           // xBOOL = typedef char
    uint8_t num_frames;
    uint8_t effect[2];
    int16_t origin_x;
    int16_t origin_y;
    int16_t width;
    int16_t height;
    int16_t effect_origin_x;
    int16_t effect_origin_y;
    int16_t selection_origin_x;
    int16_t selection_origin_y;
    int16_t selection_size;
    int16_t effect_type;
    int16_t effect_activation;
} ani_type_on_disk; // 26 bytes

typedef struct
{
    char used;
    uint8_t num_frames;
    uint8_t effect[2];
    int16_t origin_x;
    int16_t origin_y;
    int16_t width;
    int16_t height;
    int16_t anim_offset_x;
    int16_t anim_offset_y;
    int16_t anim_width;
    int16_t anim_height;
} ani_type_on_disk_old; // 20 bytes

#pragma pack(pop)

_Static_assert(sizeof(ani_type_on_disk) == 26, "ani_type_on_disk must be 26 bytes");
_Static_assert(sizeof(ani_type_on_disk_old) == 20, "ani_type_on_disk_old must be 20 bytes");

// ---------------------------------------------------------------------------
// Parser
// ---------------------------------------------------------------------------

bool ani_parse(const uint8_t* data, uint32_t size, ani_info* out)
{
    if (!data || !out) return false;
    memset(out, 0, sizeof(*out));

    uint32_t new_size = ANI_TYPE_COUNT * sizeof(ani_type_on_disk);     // 234
    uint32_t old_size = ANI_TYPE_COUNT * sizeof(ani_type_on_disk_old); // 180

    if (size >= new_size)
    {
        const ani_type_on_disk* src = (const ani_type_on_disk*)data;
        for (int i = 0; i < ANI_TYPE_COUNT; ++i)
        {
            out->types[i].used = src[i].used != 0;
            out->types[i].num_frames = src[i].num_frames;
            out->types[i].origin_x = src[i].origin_x;
            out->types[i].origin_y = src[i].origin_y;
            out->types[i].width = src[i].width;
            out->types[i].height = src[i].height;
        }
        return true;
    }
    else if (size >= old_size)
    {
        const ani_type_on_disk_old* src = (const ani_type_on_disk_old*)data;
        for (int i = 0; i < ANI_TYPE_COUNT; ++i)
        {
            out->types[i].used = src[i].used != 0;
            out->types[i].num_frames = src[i].num_frames;
            out->types[i].origin_x = src[i].origin_x;
            out->types[i].origin_y = src[i].origin_y;
            out->types[i].width = src[i].width;
            out->types[i].height = src[i].height;
        }
        return true;
    }

    fprintf(stderr, "[ani] Unexpected ANI size: %u (expected %u or %u)\n", size, new_size, old_size);
    return false;
}

// ---------------------------------------------------------------------------
// Animation player
// ---------------------------------------------------------------------------

void anim_player_set(anim_player* p, const ani_info* info, uint8_t anim_type, uint8_t direction)
{
    p->anim_type = anim_type;
    p->direction = direction;
    p->frame = 0;
    p->timer = 0.0f;
    p->frame_duration = 0.1f; // 10 FPS default
    if (anim_type < ANI_TYPE_COUNT && info->types[anim_type].used)
        p->num_frames = info->types[anim_type].num_frames;
    else
        p->num_frames = 1;
}

bool anim_player_update(anim_player* p, float dt)
{
    if (p->num_frames <= 1) return false;
    p->timer += dt;
    bool cycled = false;
    while (p->timer >= p->frame_duration)
    {
        p->timer -= p->frame_duration;
        p->frame++;
        if (p->frame >= p->num_frames)
        {
            p->frame = 0;
            cycled = true;
        }
    }
    return cycled;
}

void anim_player_uv(const anim_player* p, const ani_type* type,
                     uint32_t tex_width, uint32_t tex_height,
                     float uv_offset[2], float uv_scale[2])
{
    float fw = (float)type->width;
    float fh = (float)type->height;
    float tw = (float)tex_width;
    float th = (float)tex_height;

    // Clamp direction/frame to what actually fits in the sheet
    uint32_t max_dirs = (fh > 0) ? (uint32_t)(th / fh) : 1;
    uint32_t max_frames = (fw > 0) ? (uint32_t)(tw / fw) : 1;
    uint32_t dir = p->direction < max_dirs ? p->direction : 0;
    uint32_t frame = p->frame < max_frames ? p->frame : 0;

    // Inset by one texel to prevent sampling adjacent frames
    float tx = 1.0f / tw;
    float ty = 1.0f / th;

    // Sprite sheet layout: sourceX = frame * width, sourceY = direction * height
    uv_offset[0] = (frame * fw) / tw + tx;
    uv_offset[1] = (dir * fh) / th + ty;
    uv_scale[0] = fw / tw - tx * 2.0f;
    uv_scale[1] = fh / th - ty * 2.0f;
}
