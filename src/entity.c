#include "entity.h"
#include "gfx.h"
#include "xcr.h"
#include "image.h"
#include "map.h"
#include "pathfind.h"
#include "audio.h"

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

// Convert pixel position to cell coordinates
static uint16_t px_to_cell_x(float px) { return (uint16_t)((int)(px / CELL_W)); }
static uint16_t px_to_cell_y(float py) { return (uint16_t)((int)(py / CELL_H)); }

// Convert cell to pixel (cell center)
static float cell_to_px_x(uint16_t cx) { return (float)cx * CELL_W + CELL_W * 0.5f; }
static float cell_to_px_y(uint16_t cy) { return (float)cy * CELL_H + CELL_H * 0.5f; }

// Move unit along its current path. Returns true if still walking.
static bool follow_path(unit* u, float dt)
{
    if (u->path.current >= u->path.length)
        return false;

    // Advance through waypoints that are already close enough.
    // Use a generous threshold so separation forces don't prevent progress.
    // For intermediate waypoints, use half a cell width; for the final one, use a tighter check.
    while (u->path.current < u->path.length)
    {
        float tx = cell_to_px_x(u->path.points[u->path.current].x);
        float ty = cell_to_px_y(u->path.points[u->path.current].y);
        float dx = tx - u->x;
        float dy = ty - u->y;
        float dist2 = dx * dx + dy * dy;

        bool is_final = (u->path.current == u->path.length - 1);
        float threshold = is_final ? 6.0f : (float)(CELL_W / 2);

        if (dist2 <= threshold * threshold)
        {
            u->path.current++;
        }
        else
        {
            break;
        }
    }

    if (u->path.current >= u->path.length)
        return false; // path complete

    float tx = cell_to_px_x(u->path.points[u->path.current].x);
    float ty = cell_to_px_y(u->path.points[u->path.current].y);
    float dx = tx - u->x;
    float dy = ty - u->y;
    float dist = sqrtf(dx * dx + dy * dy);
    float step = u->speed * dt;

    // Update facing direction
    if (dist > 0.5f)
    {
        uint8_t dir = direction_from_delta(dx, dy);
        if (dir != u->anim.direction)
            set_anim_with_fallback(u, ANI_WALK, ANI_STAND, dir);
    }

    if (dist <= step)
    {
        u->x = tx;
        u->y = ty;
    }
    else
    {
        u->x += (dx / dist) * step;
        u->y += (dy / dist) * step;
    }
    return true;
}

// Issue a pathfound move command for a unit
static bool unit_move_to(unit* u, const game_map* map, float world_x, float world_y)
{
    uint16_t sx = px_to_cell_x(u->x);
    uint16_t sy = px_to_cell_y(u->y);
    uint16_t dx = px_to_cell_x(world_x);
    uint16_t dy = px_to_cell_y(world_y);

    if (!pathfind_find(map, sx, sy, dx, dy, u->move_mode, &u->path))
        return false;

    u->state = UNIT_STATE_WALKING;
    u->path.current = 0;
    // Skip first waypoint if it's our current cell
    if (u->path.length > 1 && u->path.points[0].x == sx && u->path.points[0].y == sy)
        u->path.current = 1;

    float next_x = cell_to_px_x(u->path.points[u->path.current].x);
    float next_y = cell_to_px_y(u->path.points[u->path.current].y);
    uint8_t dir = direction_from_delta(next_x - u->x, next_y - u->y);
    set_anim_with_fallback(u, ANI_WALK, ANI_STAND, dir);
    return true;
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

void units_update(unit_array* arr, const game_map* map, float dt, const combat_sounds* sounds)
{
    for (uint32_t i = 0; i < arr->count; ++i)
    {
        unit* u = &arr->units[i];

        switch (u->state)
        {
            case UNIT_STATE_IDLE:
            case UNIT_STATE_WALKING:
            {
                // Validate existing combat target
                if (u->target_unit != UINT32_MAX &&
                    (u->target_unit >= arr->count ||
                     arr->units[u->target_unit].state == UNIT_STATE_DYING ||
                     arr->units[u->target_unit].team == u->team))
                {
                    u->target_unit = UINT32_MAX;
                }

                // AI auto-aggro: only scan for enemies when IDLE (not during manual move)
                if (u->state == UNIT_STATE_IDLE && u->target_unit == UINT32_MAX)
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
                        // Repath periodically when chasing
                        u->repath_timer -= dt;
                        if (u->state != UNIT_STATE_WALKING || u->repath_timer <= 0.0f)
                        {
                            u->repath_timer = 0.5f;
                            unit_move_to(u, map, target->x, target->y);
                        }
                    }
                }

                // Movement: follow path
                if (u->state == UNIT_STATE_WALKING)
                {
                    if (!follow_path(u, dt))
                    {
                        // Path finished
                        if (u->target_unit == UINT32_MAX)
                        {
                            u->state = UNIT_STATE_IDLE;
                            anim_player_set(&u->anim, &u->type->ani, ANI_STAND, u->anim.direction);
                        }
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
                    u->repath_timer = 0.0f; // force immediate repath
                    unit_move_to(u, map, target->x, target->y);
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

                        if (sounds && sounds->snd_attack != UINT32_MAX)
                            audio_play(sounds->snd_attack, 0.3f);

                        if (target->health <= 0)
                        {
                            if (sounds && sounds->snd_die != UINT32_MAX)
                                audio_play(sounds->snd_die, 0.5f);
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

    // Soft unit separation — gentle push to prevent stacking.
    // Must be weaker than movement speed (80 px/s) so path-following wins.
    const float sep_radius = 18.0f;
    const float sep_strength = 30.0f; // pixels/sec (well below movement speed)
    for (uint32_t i = 0; i < arr->count; ++i)
    {
        unit* a = &arr->units[i];
        if (a->state == UNIT_STATE_DYING) continue;
        for (uint32_t j = i + 1; j < arr->count; ++j)
        {
            unit* b = &arr->units[j];
            if (b->state == UNIT_STATE_DYING) continue;

            float dx = b->x - a->x;
            float dy = b->y - a->y;
            float dist2 = dx * dx + dy * dy;
            if (dist2 >= sep_radius * sep_radius || dist2 < 0.01f) continue;

            float dist = sqrtf(dist2);
            float overlap = sep_radius - dist;
            float push = overlap * sep_strength * dt / dist;
            a->x -= dx * push * 0.5f;
            a->y -= dy * push * 0.5f;
            b->x += dx * push * 0.5f;
            b->y += dy * push * 0.5f;
        }
    }
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------

// Y-sorted draw order so units further down screen render on top (isometric depth)
static const unit_array* s_sort_arr;
static int compare_unit_y(const void* a, const void* b)
{
    uint32_t ia = *(const uint32_t*)a;
    uint32_t ib = *(const uint32_t*)b;
    float ya = s_sort_arr->units[ia].y;
    float yb = s_sort_arr->units[ib].y;
    return (ya > yb) - (ya < yb);
}

void units_draw(const unit_array* arr, uint32_t white_tex)
{
    // Build sorted index array
    uint32_t order[MAX_UNITS];
    for (uint32_t i = 0; i < arr->count; ++i)
        order[i] = i;
    s_sort_arr = arr;
    qsort(order, arr->count, sizeof(uint32_t), compare_unit_y);

    for (uint32_t idx = 0; idx < arr->count; ++idx)
    {
        const unit* u = &arr->units[order[idx]];
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
