#include "gfx.h"
#include "xcr.h"
#include "image.h"
#include "entity.h"

#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Terrain loading
// ---------------------------------------------------------------------------

static uint32_t terrain_tex = UINT32_MAX;
static float terrain_tile_w = 256.0f;
static float terrain_tile_h = 256.0f;

static void load_terrain(const char* sides_path)
{
    // Derive Assets/Terrain/ path from the Sides path
    // E.g., "C:/Games/WBC3/Assets/Sides" -> "C:/Games/WBC3/Assets/Terrain/Grass.xcr"
    char terrain_path[512];
    snprintf(terrain_path, sizeof(terrain_path), "%s", sides_path);

    // Strip trailing slash
    size_t len = strlen(terrain_path);
    while (len > 0 && (terrain_path[len - 1] == '/' || terrain_path[len - 1] == '\\'))
        terrain_path[--len] = '\0';

    // Go up one directory (remove "Sides" or similar)
    char* last_sep = strrchr(terrain_path, '/');
    if (!last_sep) last_sep = strrchr(terrain_path, '\\');
    if (last_sep) *last_sep = '\0';

    // Append Terrain/Grass.xcr
    char xcr_path[512];
    snprintf(xcr_path, sizeof(xcr_path), "%s/Terrain/Grass.xcr", terrain_path);

    xcr_archive* archive = xcr_load(xcr_path);
    if (!archive)
    {
        fprintf(stderr, "[main] Terrain XCR not found: %s\n", xcr_path);
        return;
    }

    // Load the first valid BMP
    for (uint32_t i = 0; i < archive->resource_count; ++i)
    {
        if (archive->resources[i].type != XCR_RESOURCE_BMP) continue;
        image img = {0};
        if (image_decode_bmp(archive->resources[i].data, archive->resources[i].size, &img))
        {
            terrain_tex = gfx_create_texture(img.width, img.height, img.pixels);
            terrain_tile_w = (float)img.width;
            terrain_tile_h = (float)img.height;
            fprintf(stderr, "[main] Loaded terrain tile: %ux%u\n", img.width, img.height);
            image_free(&img);
            break;
        }
    }
    xcr_free(archive);
}

// ---------------------------------------------------------------------------
// Box selection
// ---------------------------------------------------------------------------

static bool  box_selecting = false;
static float box_start_x, box_start_y;

static void handle_selection(unit_array* units, const gfx_input* input, float cam_x, float cam_y)
{
    if (input->mouse_left_pressed)
    {
        box_selecting = true;
        box_start_x = input->mouse_x;
        box_start_y = input->mouse_y;
    }

    if (input->mouse_left_released && box_selecting)
    {
        box_selecting = false;

        float min_sx = fminf(box_start_x, input->mouse_x);
        float max_sx = fmaxf(box_start_x, input->mouse_x);
        float min_sy = fminf(box_start_y, input->mouse_y);
        float max_sy = fmaxf(box_start_y, input->mouse_y);

        float min_wx = min_sx - cam_x, max_wx = max_sx - cam_x;
        float min_wy = min_sy - cam_y, max_wy = max_sy - cam_y;

        bool is_click = (max_sx - min_sx < 4.0f && max_sy - min_sy < 4.0f);

        for (uint32_t i = 0; i < units->count; ++i)
            units->units[i].selected = false;

        if (is_click)
        {
            for (uint32_t i = 0; i < units->count; ++i)
            {
                unit* u = &units->units[i];
                if (u->team != 0 || u->state == UNIT_STATE_DYING) continue;

                const ani_type* at = &u->type->ani.types[u->anim.anim_type];
                float left   = u->x - (float)at->origin_x;
                float top    = u->y - (float)at->origin_y;
                float right  = left + (float)at->width;
                float bottom = top + (float)at->height;

                if (min_wx >= left && min_wx <= right && min_wy >= top && min_wy <= bottom)
                {
                    u->selected = true;
                    break;
                }
            }
        }
        else
        {
            for (uint32_t i = 0; i < units->count; ++i)
            {
                unit* u = &units->units[i];
                if (u->team != 0 || u->state == UNIT_STATE_DYING) continue;
                if (u->x >= min_wx && u->x <= max_wx && u->y >= min_wy && u->y <= max_wy)
                    u->selected = true;
            }
        }
    }
}

static void handle_move_command(unit_array* units, const gfx_input* input, float cam_x, float cam_y)
{
    if (!input->mouse_right_pressed) return;

    float world_x = input->mouse_x - cam_x;
    float world_y = input->mouse_y - cam_y;

    for (uint32_t i = 0; i < units->count; ++i)
    {
        unit* u = &units->units[i];
        if (!u->selected || u->team != 0 || u->state == UNIT_STATE_DYING) continue;

        u->target_x = world_x;
        u->target_y = world_y;
        u->target_unit = UINT32_MAX; // clear combat target on manual move
        u->state = UNIT_STATE_WALKING;

        float dx = world_x - u->x;
        float dy = world_y - u->y;
        float angle = atan2f(dx, -dy);
        if (angle < 0.0f) angle += 2.0f * 3.14159265f;
        uint8_t dir = (uint8_t)((int)(angle / (2.0f * 3.14159265f) * 8.0f + 0.5f) % 8);

        uint8_t walk_anim = (u->type->tex[ANI_WALK] != UINT32_MAX) ? ANI_WALK : ANI_STAND;
        anim_player_set(&u->anim, &u->type->ani, walk_anim, dir);
    }
}

static void draw_selection_box(const gfx_input* input, uint32_t white_tex)
{
    if (!box_selecting || !input->mouse_left_held) return;

    float cam_x, cam_y;
    gfx_get_camera(&cam_x, &cam_y);

    // Subtract camera because the shader adds it back
    float x0 = box_start_x - cam_x;
    float y0 = box_start_y - cam_y;
    float x1 = input->mouse_x - cam_x;
    float y1 = input->mouse_y - cam_y;

    float min_x = fminf(x0, x1), max_x = fmaxf(x0, x1);
    float min_y = fminf(y0, y1), max_y = fmaxf(y0, y1);
    float w = max_x - min_x;
    float h = max_y - min_y;
    float t = 2.0f;

    uint32_t green = 0xFF00FF00;
    gfx_draw_sprite(min_x, min_y, w, t, white_tex, green);         // top
    gfx_draw_sprite(min_x, max_y - t, w, t, white_tex, green);     // bottom
    gfx_draw_sprite(min_x, min_y, t, h, white_tex, green);         // left
    gfx_draw_sprite(max_x - t, min_y, t, h, white_tex, green);     // right
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: game <path-to-wbc3-sides-folder>\n");
        return 1;
    }

    const char* data_path = argv[1];

    gfx_config config = {
        .window_title = "Open Battlecry",
        .window_width = 1280,
        .window_height = 720,
        .enable_validation = true,
    };

    if (!gfx_init(&config))
        return 1;

    // 1x1 white pixel texture (for health bars and selection box)
    uint8_t white_pixel[4] = {255, 255, 255, 255};
    uint32_t white_tex = gfx_create_texture(1, 1, white_pixel);

    // Load terrain
    load_terrain(data_path);
    if (terrain_tex == UINT32_MAX)
    {
        // Fallback procedural grass
        uint8_t grass[16 * 16 * 4];
        for (int py = 0; py < 16; py++)
            for (int px = 0; px < 16; px++)
            {
                int idx = (py * 16 + px) * 4;
                grass[idx + 0] = 30;
                grass[idx + 1] = (uint8_t)(60 + ((px * 7 + py * 13) % 20));
                grass[idx + 2] = 20;
                grass[idx + 3] = 255;
            }
        terrain_tex = gfx_create_texture(16, 16, grass);
        terrain_tile_w = 256.0f;
        terrain_tile_h = 256.0f;
    }

    // Load unit types from XCR
    const char* xcr_names[] = {"DarkDwarves.xcr", "Knights.xcr", "Barbarians.xcr", "Undead.xcr", NULL};
    xcr_archive* archive = NULL;
    char xcr_path[512];
    for (int i = 0; xcr_names[i] && !archive; ++i)
    {
        snprintf(xcr_path, sizeof(xcr_path), "%s/%s", data_path, xcr_names[i]);
        archive = xcr_load(xcr_path);
    }
    if (!archive)
    {
        fprintf(stderr, "[main] No XCR archive found in: %s\n", data_path);
        gfx_shutdown();
        return 1;
    }

    // Find up to 3 unit types with walk animations
    #define MAX_UNIT_TYPES 4
    unit_type unit_types[MAX_UNIT_TYPES];
    uint32_t num_unit_types = 0;

    for (uint32_t i = 0; i < archive->resource_count && num_unit_types < 3; ++i)
    {
        const xcr_resource* res = &archive->resources[i];
        if (res->type != XCR_RESOURCE_ANI) continue;

        char base_code[8] = {0};
        const char* dot = strstr(res->name, ".");
        if (!dot || dot == res->name) continue;
        size_t len = (size_t)(dot - res->name);
        if (len >= sizeof(base_code)) continue;
        memcpy(base_code, res->name, len);
        base_code[len] = '\0';

        // Skip duplicates
        bool dup = false;
        for (uint32_t j = 0; j < num_unit_types; ++j)
            if (strcmp(unit_types[j].base_code, base_code) == 0) { dup = true; break; }
        if (dup) continue;

        unit_type ut = {0};
        if (unit_type_load(&ut, archive, base_code) && ut.tex[ANI_WALK] != UINT32_MAX)
        {
            unit_types[num_unit_types++] = ut;
        }
    }

    if (num_unit_types == 0)
    {
        fprintf(stderr, "[main] Could not load any animated unit type\n");
        xcr_free(archive);
        gfx_shutdown();
        return 1;
    }

    // Set combat stats
    unit_type_set_default_stats(&unit_types[0], 100, 12, 3, 60, 50.0f, 300.0f, 1.0f);
    if (num_unit_types > 1)
        unit_type_set_default_stats(&unit_types[1], 80, 15, 2, 50, 50.0f, 300.0f, 1.2f);
    if (num_unit_types > 2)
        unit_type_set_default_stats(&unit_types[2], 120, 10, 5, 55, 50.0f, 300.0f, 0.8f);

    // Spawn two teams
    unit_array units = {0};

    // Player team (left side)
    for (int i = 0; i < 5; ++i)
        unit_spawn(&units, &unit_types[0], 200.0f + (float)i * 60.0f, 250.0f + (float)i * 30.0f, 0);

    // Enemy team (right side)
    const unit_type* enemy_type = num_unit_types > 1 ? &unit_types[1] : &unit_types[0];
    for (int i = 0; i < 5; ++i)
        unit_spawn(&units, enemy_type, 800.0f + (float)i * 60.0f, 250.0f + (float)i * 30.0f, 1);

    fprintf(stderr, "[main] Spawned %u units (%u types)\n", units.count, num_unit_types);

    float cam_x = 0, cam_y = 0;
    const float cam_speed = 8.0f;
    uint64_t last_time = SDL_GetTicksNS();

    while (gfx_poll_events())
    {
        uint64_t now = SDL_GetTicksNS();
        float dt = (float)((double)(now - last_time) / 1.0e9);
        last_time = now;
        if (dt > 0.1f) dt = 0.1f;

        // Camera
        const bool* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_LEFT])  cam_x += cam_speed;
        if (keys[SDL_SCANCODE_RIGHT]) cam_x -= cam_speed;
        if (keys[SDL_SCANCODE_UP])    cam_y += cam_speed;
        if (keys[SDL_SCANCODE_DOWN])  cam_y -= cam_speed;
        gfx_set_camera(cam_x, cam_y);

        // Input
        const gfx_input* input = gfx_get_input();
        handle_selection(&units, input, cam_x, cam_y);
        handle_move_command(&units, input, cam_x, cam_y);

        // Update
        units_update(&units, dt);

        // Render
        if (!gfx_begin_frame())
            continue;

        // Terrain
        for (int ty = -2; ty < 10; ty++)
            for (int tx = -2; tx < 14; tx++)
                gfx_draw_sprite((float)tx * terrain_tile_w, (float)ty * terrain_tile_h,
                                terrain_tile_w, terrain_tile_h, terrain_tex, 0xFFFFFFFF);

        // Units + health bars
        units_draw(&units, white_tex);

        // Selection box overlay
        draw_selection_box(input, white_tex);

        gfx_end_frame();
    }

    xcr_free(archive);
    gfx_shutdown();
    return 0;
}
