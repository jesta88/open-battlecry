#include "entity.h"
#include "gfx.h"
#include "xcr.h"
#include "image.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Animation file suffixes (from WBC3: xcAnimation.cpp)
static const char* anim_suffixes[ANI_TYPE_COUNT] = {
    ".RLE",   // ANI_STAND
    "W.RLE",  // ANI_WALK
    "F.RLE",  // ANI_FIGHT
    "D.RLE",  // ANI_DIE
    "A.RLE",  // ANI_AMBIENT
    "Z.RLE",  // ANI_SPECIAL
    "C.RLE",  // ANI_CONVERT
    "S.RLE",  // ANI_SPELL
    "I.RLE",  // ANI_INTERFACE
};

// Sentinel state for dead units awaiting removal
#define UNIT_STATE_DEAD 0xFF

// ---------------------------------------------------------------------------
// Unit type loading
// ---------------------------------------------------------------------------

bool unit_type_load(unit_type* ut, const xcr_archive* archive, const char* base_code)
{
    memset(ut, 0, sizeof(*ut));
    for (int i = 0; i < ANI_TYPE_COUNT; ++i)
        ut->tex[i] = UINT32_MAX;
    snprintf(ut->base_code, sizeof(ut->base_code), "%s", base_code);

    char name[64];
    snprintf(name, sizeof(name), "%s.ANI", base_code);
    const xcr_resource* ani_res = xcr_find(archive, name);
    if (!ani_res || !ani_parse(ani_res->data, ani_res->size, &ut->ani))
    {
        fprintf(stderr, "[entity] ANI not found or parse failed: %s\n", name);
        return false;
    }

    for (int i = 0; i < ANI_TYPE_COUNT; ++i)
    {
        if (!ut->ani.types[i].used) continue;
        snprintf(name, sizeof(name), "%s%s", base_code, anim_suffixes[i]);
        const xcr_resource* res = xcr_find(archive, name);
        if (!res) continue;

        image img = {0};
        if (!image_decode_rle(res->data, res->size, &img)) continue;
        ut->tex[i] = gfx_create_texture(img.width, img.height, img.pixels);
        ut->sheet_w[i] = img.width;
        ut->sheet_h[i] = img.height;
        image_free(&img);
    }

    bool has_stand = ut->tex[ANI_STAND] != UINT32_MAX;
    if (has_stand)
    {
        const ani_type* s = &ut->ani.types[ANI_STAND];
        fprintf(stderr, "[entity] Loaded unit type '%s': stand=%dx%d (%d frames)\n",
                base_code, s->width, s->height, s->num_frames);
    }
    return has_stand;
}

void unit_type_set_default_stats(unit_type* ut, int16_t health, int16_t damage,
                                  int16_t armor, int16_t combat,
                                  float attack_range, float aggro_range,
                                  float attack_cooldown)
{
    ut->max_health = health;
    ut->attack_damage = damage;
    ut->armor = armor;
    ut->combat_skill = combat;
    ut->attack_range = attack_range;
    ut->aggro_range = aggro_range;
    ut->attack_cooldown = attack_cooldown;
}

// ---------------------------------------------------------------------------
// Spawning
// ---------------------------------------------------------------------------

uint32_t unit_spawn(unit_array* arr, const unit_type* type, float x, float y, uint8_t team)
{
    if (arr->count >= MAX_UNITS) return UINT32_MAX;

    uint32_t idx = arr->count++;
    unit* u = &arr->units[idx];
    memset(u, 0, sizeof(*u));
    u->x = x;
    u->y = y;
    u->target_x = x;
    u->target_y = y;
    u->speed = 80.0f;
    u->state = UNIT_STATE_IDLE;
    u->team = team;
    u->health = type->max_health;
    u->attack_timer = 0.0f;
    u->target_unit = UINT32_MAX;
    u->type = type;
    anim_player_set(&u->anim, &type->ani, ANI_STAND, ANI_DIR_SOUTH);

    return idx;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static uint8_t direction_from_delta(float dx, float dy)
{
    float angle = atan2f(dx, -dy);
    if (angle < 0.0f) angle += 2.0f * 3.14159265f;
    int dir = (int)(angle / (2.0f * 3.14159265f) * 8.0f + 0.5f) % 8;
    return (uint8_t)dir;
}

static float distance_between(const unit* a, const unit* b)
{
    float dx = b->x - a->x;
    float dy = b->y - a->y;
    return sqrtf(dx * dx + dy * dy);
}

static uint32_t find_nearest_enemy(const unit_array* arr, uint32_t self_idx)
{
    const unit* self = &arr->units[self_idx];
    float best_dist = self->type->aggro_range;
    uint32_t best_idx = UINT32_MAX;

    for (uint32_t i = 0; i < arr->count; ++i)
    {
        if (i == self_idx) continue;
        const unit* other = &arr->units[i];
        if (other->team == self->team) continue;
        if (other->state == UNIT_STATE_DYING) continue;

        float dist = distance_between(self, other);
        if (dist < best_dist)
        {
            best_dist = dist;
            best_idx = i;
        }
    }
    return best_idx;
}

static void set_anim_with_fallback(unit* u, uint8_t desired, uint8_t fallback, uint8_t dir)
{
    uint8_t anim = (u->type->tex[desired] != UINT32_MAX) ? desired : fallback;
    anim_player_set(&u->anim, &u->type->ani, anim, dir);
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

void units_update(unit_array* arr, float dt)
{
    for (uint32_t i = 0; i < arr->count; ++i)
    {
        unit* u = &arr->units[i];

        switch (u->state)
        {
            case UNIT_STATE_IDLE:
            case UNIT_STATE_WALKING:
            {
                // AI: scan for enemy if no target
                if (u->target_unit == UINT32_MAX || u->target_unit >= arr->count ||
                    arr->units[u->target_unit].state == UNIT_STATE_DYING ||
                    arr->units[u->target_unit].team == u->team)
                {
                    u->target_unit = find_nearest_enemy(arr, i);
                }

                // Engage or chase target
                if (u->target_unit != UINT32_MAX)
                {
                    unit* target = &arr->units[u->target_unit];
                    float dist = distance_between(u, target);

                    if (dist <= u->type->attack_range)
                    {
                        // Start fighting
                        u->state = UNIT_STATE_FIGHTING;
                        u->attack_timer = 0.0f;
                        uint8_t dir = direction_from_delta(target->x - u->x, target->y - u->y);
                        set_anim_with_fallback(u, ANI_FIGHT, ANI_STAND, dir);
                        break;
                    }
                    else
                    {
                        // Walk toward target
                        u->target_x = target->x;
                        u->target_y = target->y;
                        if (u->state != UNIT_STATE_WALKING)
                        {
                            u->state = UNIT_STATE_WALKING;
                            uint8_t dir = direction_from_delta(target->x - u->x, target->y - u->y);
                            set_anim_with_fallback(u, ANI_WALK, ANI_STAND, dir);
                        }
                    }
                }

                // Movement (for WALKING state)
                if (u->state == UNIT_STATE_WALKING)
                {
                    float dx = u->target_x - u->x;
                    float dy = u->target_y - u->y;
                    float dist = sqrtf(dx * dx + dy * dy);
                    float step = u->speed * dt;

                    if (dist <= step)
                    {
                        u->x = u->target_x;
                        u->y = u->target_y;
                        if (u->target_unit == UINT32_MAX)
                        {
                            u->state = UNIT_STATE_IDLE;
                            anim_player_set(&u->anim, &u->type->ani, ANI_STAND, u->anim.direction);
                        }
                    }
                    else
                    {
                        u->x += (dx / dist) * step;
                        u->y += (dy / dist) * step;
                    }
                }

                anim_player_update(&u->anim, dt);
                break;
            }

            case UNIT_STATE_FIGHTING:
            {
                // Validate target
                if (u->target_unit == UINT32_MAX || u->target_unit >= arr->count ||
                    arr->units[u->target_unit].state == UNIT_STATE_DYING)
                {
                    u->target_unit = UINT32_MAX;
                    u->state = UNIT_STATE_IDLE;
                    anim_player_set(&u->anim, &u->type->ani, ANI_STAND, u->anim.direction);
                    break;
                }

                unit* target = &arr->units[u->target_unit];
                float dist = distance_between(u, target);

                // Chase if target moved out of range
                if (dist > u->type->attack_range * 1.5f)
                {
                    u->state = UNIT_STATE_WALKING;
                    u->target_x = target->x;
                    u->target_y = target->y;
                    uint8_t dir = direction_from_delta(target->x - u->x, target->y - u->y);
                    set_anim_with_fallback(u, ANI_WALK, ANI_STAND, dir);
                    break;
                }

                // Face target
                uint8_t dir = direction_from_delta(target->x - u->x, target->y - u->y);
                if (dir != u->anim.direction)
                    set_anim_with_fallback(u, ANI_FIGHT, ANI_STAND, dir);

                // Attack timing
                u->attack_timer -= dt;
                if (u->attack_timer <= 0.0f)
                {
                    u->attack_timer = u->type->attack_cooldown;

                    // Hit resolution
                    int combat_a = u->type->combat_skill;
                    int combat_b = target->type->combat_skill;
                    int hit_chance = (combat_a + combat_b > 0) ? (100 * combat_a) / (combat_a + combat_b) : 50;
                    int roll = rand() % 100 + 1;

                    if (roll <= hit_chance)
                    {
                        int damage = u->type->attack_damage - target->type->armor;
                        if (damage < 1) damage = 1;
                        target->health -= (int16_t)damage;

                        if (target->health <= 0)
                        {
                            target->state = UNIT_STATE_DYING;
                            target->target_unit = UINT32_MAX;
                            target->selected = false;
                            set_anim_with_fallback(target, ANI_DIE, ANI_STAND, target->anim.direction);
                        }
                    }
                }

                anim_player_update(&u->anim, dt);
                break;
            }

            case UNIT_STATE_DYING:
            {
                bool finished = anim_player_update(&u->anim, dt);
                if (finished)
                    u->state = UNIT_STATE_DEAD;
                break;
            }
        }
    }

    // Remove dead units and remap target indices
    uint32_t remap[MAX_UNITS];
    memset(remap, 0xFF, sizeof(remap));
    uint32_t write = 0;
    for (uint32_t read = 0; read < arr->count; ++read)
    {
        if (arr->units[read].state == UNIT_STATE_DEAD) continue;
        remap[read] = write;
        if (write != read)
            arr->units[write] = arr->units[read];
        write++;
    }
    arr->count = write;

    for (uint32_t i = 0; i < arr->count; ++i)
    {
        uint32_t t = arr->units[i].target_unit;
        arr->units[i].target_unit = (t < MAX_UNITS) ? remap[t] : UINT32_MAX;
    }
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------

void units_draw(const unit_array* arr, uint32_t white_tex)
{
    for (uint32_t i = 0; i < arr->count; ++i)
    {
        const unit* u = &arr->units[i];
        uint8_t anim_type = u->anim.anim_type;
        uint32_t tex = u->type->tex[anim_type];
        if (tex == UINT32_MAX) continue;

        const ani_type* at = &u->type->ani.types[anim_type];

        float uv_off[2], uv_sc[2];
        anim_player_uv(&u->anim, at,
                        u->type->sheet_w[anim_type], u->type->sheet_h[anim_type],
                        uv_off, uv_sc);

        float draw_x = u->x - (float)at->origin_x;
        float draw_y = u->y - (float)at->origin_y;
        float draw_w = (float)at->width;
        float draw_h = (float)at->height;

        uint32_t color;
        if (u->selected)
            color = 0xFF80FF80; // green tint
        else if (u->team == 1)
            color = 0xFF5050FF; // red tint
        else
            color = 0xFFFFFFFF;

        gfx_draw_sprite_region(draw_x, draw_y, draw_w, draw_h, tex, color,
                               uv_off[0], uv_off[1], uv_sc[0], uv_sc[1]);

        // Health bar (only when damaged, skip dying)
        if (u->state != UNIT_STATE_DYING && u->health < u->type->max_health && u->health > 0)
        {
            float bar_w = 30.0f, bar_h = 4.0f;
            float bar_x = u->x - bar_w * 0.5f;
            float bar_y = u->y - (float)at->origin_y - bar_h - 2.0f;

            float hp_frac = (float)u->health / (float)u->type->max_health;
            if (hp_frac < 0.0f) hp_frac = 0.0f;

            gfx_draw_sprite(bar_x, bar_y, bar_w, bar_h, white_tex, 0xFF0000FF); // red bg
            gfx_draw_sprite(bar_x, bar_y, bar_w * hp_frac, bar_h, white_tex, 0xFF00FF00); // green fill
        }
    }
}
