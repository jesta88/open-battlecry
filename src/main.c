#include "gfx.h"
#include "entity.h"
#include "map.h"
#include "pathfind.h"
#include "font.h"
#include "audio.h"
#include "hud.h"
#include "menu.h"
#include "resource.h"
#include "mine.h"
#include "building.h"
#include "baked_loader.h"
#include "baked_format.h"
#include "file_io.h"
#include "console.h"

#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Terrain textures (loaded from baked/ directory)
static uint32_t terrain_textures[TERRAIN_COUNT];

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

static void handle_move_command(unit_array* units, const game_map* map, const gfx_input* input, float cam_x, float cam_y)
{
    if (!input->mouse_right_pressed) return;

    float world_x = input->mouse_x - cam_x;
    float world_y = input->mouse_y - cam_y;

    for (uint32_t i = 0; i < units->count; ++i)
    {
        unit* u = &units->units[i];
        if (!u->selected || u->team != 0 || u->state == UNIT_STATE_DYING) continue;

        u->target_unit = UINT32_MAX; // clear combat target on manual move

        // Pathfind to destination — convert pixel coords to cell coords
        int src_cx = (int)(u->x / CELL_W);
        int src_cy = (int)(u->y / CELL_H);
        int dst_cx = (int)(world_x / CELL_W);
        int dst_cy = (int)(world_y / CELL_H);

        // Clamp to map bounds
        if (src_cx < 0) src_cx = 0;
        if (src_cy < 0) src_cy = 0;
        if (dst_cx < 0) dst_cx = 0;
        if (dst_cy < 0) dst_cy = 0;
        if (src_cx >= (int)map->width) src_cx = (int)map->width - 1;
        if (src_cy >= (int)map->height) src_cy = (int)map->height - 1;
        if (dst_cx >= (int)map->width) dst_cx = (int)map->width - 1;
        if (dst_cy >= (int)map->height) dst_cy = (int)map->height - 1;

        uint16_t sx = (uint16_t)src_cx;
        uint16_t sy = (uint16_t)src_cy;
        uint16_t dx = (uint16_t)dst_cx;
        uint16_t dy = (uint16_t)dst_cy;

        if (pathfind_find(map, sx, sy, dx, dy, u->move_mode, &u->path))
        {
            u->state = UNIT_STATE_WALKING;
            u->path.current = 0;
            // Skip first waypoint if it's our current cell
            if (u->path.length > 1 && u->path.points[0].x == sx && u->path.points[0].y == sy)
                u->path.current = 1;

            float next_px = (float)u->path.points[u->path.current].x * CELL_W + CELL_W * 0.5f;
            float next_py = (float)u->path.points[u->path.current].y * CELL_H + CELL_H * 0.5f;
            float ddx = next_px - u->x;
            float ddy = next_py - u->y;
            float angle = atan2f(ddx, -ddy);
            if (angle < 0.0f) angle += 2.0f * 3.14159265f;
            uint8_t dir = (uint8_t)((int)(angle / (2.0f * 3.14159265f) * 8.0f + 0.5f) % 8);

            uint8_t walk_anim = (u->type->tex[ANI_WALK] != UINT32_MAX) ? ANI_WALK : ANI_STAND;
            anim_player_set(&u->anim, &u->type->ani, walk_anim, dir);
        }
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
        fprintf(stderr, "Usage: game <baked_data_directory>\n");
        fprintf(stderr, "\nRun asset_baker first to convert WBC3 data.\n");
        return 1;
    }

    // Baked data directory (output of asset_baker)
    char baked_dir[512];
    snprintf(baked_dir, sizeof(baked_dir), "%s", argv[1]);
    size_t rlen = strlen(baked_dir);
    while (rlen > 0 && (baked_dir[rlen - 1] == '/' || baked_dir[rlen - 1] == '\\'))
        baked_dir[--rlen] = '\0';

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

    // Font
    font ui_font = {0};
    font title_font = {0};
    {
        // Construct font path relative to executable
        char font_path[512];
        const char* base = SDL_GetBasePath();
        snprintf(font_path, sizeof(font_path), "%sassets/fonts/Inconsolata-Regular.ttf", base ? base : "");
        if (!font_load(&ui_font, font_path, 16.0f))
            fprintf(stderr, "[main] Warning: UI font not loaded\n");
        if (!font_load(&title_font, font_path, 32.0f))
            fprintf(stderr, "[main] Warning: title font not loaded\n");
    }

    // Audio
    audio_init();

    // HUD
    hud_init(&(hud_config){.ui_font = &ui_font, .white_tex = white_tex});

    // Menu
    menu_init(&(menu_config){.ui_font = &ui_font, .title_font = &title_font, .white_tex = white_tex});

    // Map (procedural for now — scenario loading comes later)
    game_map map = {0};
    if (!map_init(&map, 128, 128))
    {
        fprintf(stderr, "[main] Failed to allocate map\n");
        gfx_shutdown();
        return 1;
    }
    map_generate_test(&map);
    fprintf(stderr, "[main] Map initialized: %ux%u cells\n", map.width, map.height);

    // Load terrain textures from baked BMPs
    {
        char terrain_dir[512];
        snprintf(terrain_dir, sizeof(terrain_dir), "%s/terrain", baked_dir);
        baked_load_terrain(terrain_dir, terrain_textures);
    }

    // Load unit definitions from baked binary
    baked_unit_def* unit_defs = NULL;
    uint32_t num_unit_defs = 0;
    {
        char units_bin[512];
        snprintf(units_bin, sizeof(units_bin), "%s/data/units.bin", baked_dir);
        baked_load_units(units_bin, &unit_defs, &num_unit_defs);
    }

    // Load unit type sprites from first available side
    #define MAX_UNIT_TYPES 32
    unit_type unit_types[MAX_UNIT_TYPES];
    uint32_t num_unit_types = 0;

    {
        // Load units from a single side for the demo
        // Try sides in order until we get enough units with walk animations
        static const char* side_names[] = {"knights", "darkdwarves", "barbarians", "undead",
                                           "orcs", "empire", "highelves", NULL};
        char active_side_dir[512] = {0};

        for (int s = 0; side_names[s] && num_unit_types < 3; s++)
        {
            snprintf(active_side_dir, sizeof(active_side_dir), "%s/sides/%s", baked_dir, side_names[s]);

            // Scan the baked unit defs for codes, try loading each from this side
            for (uint32_t d = 0; d < num_unit_defs && num_unit_types < MAX_UNIT_TYPES; d++)
            {
                // Skip if we already have this code
                bool dup = false;
                for (uint32_t j = 0; j < num_unit_types; j++)
                    if (strcmp(unit_types[j].base_code, unit_defs[d].code) == 0) { dup = true; break; }
                if (dup) continue;

                unit_type ut = {0};
                if (baked_load_unit_type(&ut, active_side_dir, unit_defs[d].code))
                {
                    if (ut.tex[ANI_WALK] != UINT32_MAX)
                    {
                        // Apply stats from baked definitions
                        ut.max_health = (int16_t)unit_defs[d].hits;
                        ut.attack_damage = (int16_t)unit_defs[d].damage;
                        ut.armor = (int16_t)unit_defs[d].armor;
                        ut.combat_skill = (int16_t)unit_defs[d].combat;
                        ut.attack_range = (float)unit_defs[d].damage_range;
                        ut.aggro_range = 300.0f;
                        ut.attack_cooldown = 1.0f;
                        unit_types[num_unit_types++] = ut;
                        fprintf(stderr, "[main] Loaded unit: %s '%s' HP=%d DMG=%d\n",
                                ut.base_code, unit_defs[d].name,
                                ut.max_health, ut.attack_damage);
                    }
                }
            }

            // Stop once we have at least 3 from one side
            if (num_unit_types >= 3) break;

            // Reset if this side didn't work
            if (num_unit_types == 0)
                active_side_dir[0] = '\0';
        }
    }

    if (num_unit_types == 0)
    {
        fprintf(stderr, "[main] Could not load any unit types from baked data\n");
        gfx_shutdown();
        return 1;
    }

    // Load sounds from baked WAV files
    uint32_t snd_attack = UINT32_MAX;
    uint32_t snd_die = UINT32_MAX;
    uint32_t snd_select = UINT32_MAX;
    uint32_t snd_move = UINT32_MAX;
    {
        char wav_path[512];
        // Try to load sounds by known names from SoundFX archive
        // The baker extracted WAVs by their original resource names
        static const struct { const char* name; uint32_t* target; } sound_map[] = {
            {"swordhit1.wav", NULL},  // loaded below
            {"swordhit2.wav", NULL},
        };
        (void)sound_map;

        // Load first few available sounds as generic sound slots
        snprintf(wav_path, sizeof(wav_path), "%s/sounds/sfx", baked_dir);
        // Try known sound filenames from the baked SoundFX archive
        static const char* try_sounds[] = {
            "goodselect.wav", "newclick.wav", "attack00.wav", "attack01.wav",
            "death1.wav", "death2.wav", NULL
        };
        for (int i = 0; try_sounds[i]; i++)
        {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/sounds/sfx/%s", baked_dir, try_sounds[i]);
            uint32_t h = baked_load_sound(full_path);
            if (h == UINT32_MAX) continue;

            if (snd_select == UINT32_MAX) { snd_select = h; snd_move = h; }
            else if (snd_attack == UINT32_MAX) snd_attack = h;
            else if (snd_die == UINT32_MAX) { snd_die = h; break; }
        }
        fprintf(stderr, "[main] Sounds loaded: select=%u move=%u attack=%u die=%u\n",
                snd_select, snd_move, snd_attack, snd_die);
    }

    // Resources
    resource_bank banks[MAX_TEAMS] = {0};
    resource_init(&banks[0], 500, 200, 200, 200); // player
    resource_init(&banks[1], 500, 200, 200, 200); // enemy

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

    // Buildings
    building_type building_types[BTYPE_COUNT];
    building_types_init_defaults(building_types);
    building_array buildings = {0};
    building_array_init(&buildings);

    // Place starting town halls
    building_place(&buildings, &map, NULL, building_types, BTYPE_TOWN_HALL, 4, 8, 0);  // player
    building_place(&buildings, &map, NULL, building_types, BTYPE_TOWN_HALL, 60, 8, 1); // enemy

    // Mines
    mine_array mines = {0};
    mine_array_init(&mines);
    mine_place(&mines, &map, 20, 10, RES_GOLD, 5);
    mine_place(&mines, &map, 40, 10, RES_GOLD, 5);
    mine_place(&mines, &map, 30, 25, RES_GOLD, 5);
    mine_place(&mines, &map, 15, 20, RES_METAL, 3);
    mine_place(&mines, &map, 50, 20, RES_METAL, 3);
    mine_place(&mines, &map, 25, 5, RES_CRYSTAL, 2);
    mine_place(&mines, &map, 45, 5, RES_STONE, 2);

    // Claim mines near starting town halls
    mines_claim_nearby(&mines, 5, 9, 10, 0);
    mines_claim_nearby(&mines, 61, 9, 10, 1);

    // Input mode
    typedef enum { INPUT_MODE_NORMAL = 0, INPUT_MODE_PLACING = 1 } input_mode;
    input_mode current_input_mode = INPUT_MODE_NORMAL;
    uint8_t placing_type_index = 0;
    int32_t selected_building = -1;

    // Minimap: create a terrain overview texture (1 pixel per cell)
    uint32_t minimap_tex = UINT32_MAX;
    {
        static const uint8_t terrain_colors[TERRAIN_COUNT][3] = {
            { 50, 120,  40}, // GRASS
            {130,  90,  50}, // DIRT
            {200, 180, 100}, // SAND
            { 30,  60, 160}, // WATER
            {128, 128, 128}, // ROCK
            {200,  60,  20}, // LAVA
            { 40,  80,  50}, // MARSH
            { 80, 140, 180}, // FORD
            {220, 225, 230}, // SNOW
            { 10,  10,  10}, // VOID
            { 80,  75,  70}, // MOUNTAIN
            {160, 140, 110}, // WALLS
            {140,  30,  30}, // IMPASSABLE
        };
        uint8_t* mm_pixels = malloc((size_t)map.width * map.height * 4);
        if (mm_pixels)
        {
            for (uint32_t y = 0; y < map.height; y++)
                for (uint32_t x = 0; x < map.width; x++)
                {
                    uint8_t t = map_get(&map, x, y).terrain;
                    if (t >= TERRAIN_COUNT) t = TERRAIN_VOID;
                    size_t idx = ((size_t)y * map.width + x) * 4;
                    mm_pixels[idx + 0] = terrain_colors[t][0];
                    mm_pixels[idx + 1] = terrain_colors[t][1];
                    mm_pixels[idx + 2] = terrain_colors[t][2];
                    mm_pixels[idx + 3] = 255;
                }
            minimap_tex = gfx_create_texture(map.width, map.height, mm_pixels);
            free(mm_pixels);
        }
    }
    const float minimap_scale = 1.5f; // display pixels per map cell
    const float minimap_margin = 8.0f;

    float cam_x = 0, cam_y = 0;
    static float cam_speed = 500.0f;          // pixels per second
    static float edge_scroll_zone = 20.0f;
    static float game_speed = 1.0f;           // time scale multiplier
    uint64_t last_time = SDL_GetTicksNS();
    game_state state = GAME_STATE_MAIN_MENU;

    // Dev console
    con_init(&(con_config){
        .ui_font = &ui_font,
        .white_tex = white_tex,
        .ctx = {
            .units = &units,
            .unit_types = unit_types,
            .num_unit_types = num_unit_types,
            .map = &map,
            .buildings = &buildings,
            .banks = banks,
            .mines = &mines,
        }
    });
    con_register_var_float("cam_speed", &cam_speed, "Camera scroll speed (px/s)");
    con_register_var_float("edge_scroll_zone", &edge_scroll_zone, "Mouse edge scroll threshold (px)");
    con_register_var_float("game_speed", &game_speed, "Time scale multiplier");

    while (gfx_poll_events())
    {
        uint64_t now = SDL_GetTicksNS();
        float dt = (float)((double)(now - last_time) / 1.0e9);
        last_time = now;
        if (dt > 0.1f) dt = 0.1f;

        const gfx_input* input = gfx_get_input();
        uint32_t screen_w, screen_h;
        gfx_get_extent(&screen_w, &screen_h);

        // Dev console (consumes input when open)
        bool console_active = con_update(input);

        // State transitions
        if (!console_active)
        {
            if (state == GAME_STATE_PLAYING && input->key_escape_pressed)
            {
                state = GAME_STATE_PAUSED;
            }
            else if (state != GAME_STATE_PLAYING)
            {
                menu_action action = menu_update(state, input);
                switch (action)
                {
                    case MENU_ACTION_PLAY:   state = GAME_STATE_PLAYING; break;
                    case MENU_ACTION_RESUME: state = GAME_STATE_PLAYING; break;
                    case MENU_ACTION_QUIT:   goto done;
                    default: break;
                }
            }
        }

        // Game update (only when playing)
        if (state == GAME_STATE_PLAYING)
        {
            float sim_dt = dt * game_speed;

            if (!console_active)
            {
                // Camera movement (keyboard + mouse edge scrolling)
                float cam_dx = 0, cam_dy = 0;
                const bool* keys = SDL_GetKeyboardState(NULL);
                if (keys[SDL_SCANCODE_LEFT])  cam_dx += 1.0f;
                if (keys[SDL_SCANCODE_RIGHT]) cam_dx -= 1.0f;
                if (keys[SDL_SCANCODE_UP])    cam_dy += 1.0f;
                if (keys[SDL_SCANCODE_DOWN])  cam_dy -= 1.0f;

                // Mouse edge scrolling
                if (input->mouse_x < edge_scroll_zone)                    cam_dx += 1.0f;
                if (input->mouse_x > (float)screen_w - edge_scroll_zone)  cam_dx -= 1.0f;
                if (input->mouse_y < edge_scroll_zone)                    cam_dy += 1.0f;
                if (input->mouse_y > (float)screen_h - edge_scroll_zone)  cam_dy -= 1.0f;

                cam_x += cam_dx * cam_speed * dt;
                cam_y += cam_dy * cam_speed * dt;

                // Clamp camera to map bounds (cam values are negative offsets)
                float max_cam_x = 0.0f;
                float min_cam_x = -((float)(map.width * CELL_W) - (float)screen_w);
                float max_cam_y = 0.0f;
                float min_cam_y = -((float)(map.height * CELL_H) - (float)screen_h);
                if (min_cam_x > max_cam_x) min_cam_x = max_cam_x;
                if (min_cam_y > max_cam_y) min_cam_y = max_cam_y;
                if (cam_x > max_cam_x) cam_x = max_cam_x;
                if (cam_x < min_cam_x) cam_x = min_cam_x;
                if (cam_y > max_cam_y) cam_y = max_cam_y;
                if (cam_y < min_cam_y) cam_y = min_cam_y;

                gfx_set_camera(cam_x, cam_y);

                // Minimap click
                bool click_on_minimap = false;
                if (minimap_tex != UINT32_MAX && input->mouse_left_pressed)
                {
                    float mm_w = (float)map.width * minimap_scale;
                    float mm_h = (float)map.height * minimap_scale;
                    float mm_screen_x = (float)screen_w - mm_w - minimap_margin;
                    float mm_screen_y = (float)screen_h - mm_h - minimap_margin;

                    if (input->mouse_x >= mm_screen_x && input->mouse_x <= mm_screen_x + mm_w &&
                        input->mouse_y >= mm_screen_y && input->mouse_y <= mm_screen_y + mm_h)
                    {
                        float frac_x = (input->mouse_x - mm_screen_x) / mm_w;
                        float frac_y = (input->mouse_y - mm_screen_y) / mm_h;
                        cam_x = -(frac_x * (float)(map.width * CELL_W) - (float)screen_w * 0.5f);
                        cam_y = -(frac_y * (float)(map.height * CELL_H) - (float)screen_h * 0.5f);
                        click_on_minimap = true;
                    }
                }

                // Building placement mode
                if (current_input_mode == INPUT_MODE_NORMAL)
                {
                    if (input->key_b_pressed)
                    {
                        current_input_mode = INPUT_MODE_PLACING;
                        placing_type_index = BTYPE_BARRACKS;
                    }
                }
                else if (current_input_mode == INPUT_MODE_PLACING)
                {
                    if (input->key_escape_pressed || input->mouse_right_pressed)
                    {
                        current_input_mode = INPUT_MODE_NORMAL;
                    }
                    else if (input->mouse_left_pressed && !click_on_minimap)
                    {
                        float world_x = input->mouse_x - cam_x;
                        float world_y = input->mouse_y - cam_y;
                        uint32_t cx = (uint32_t)(world_x / CELL_W);
                        uint32_t cy = (uint32_t)(world_y / CELL_H);
                        if (building_place(&buildings, &map, &banks[0], building_types,
                                           placing_type_index, cx, cy, 0) != UINT32_MAX)
                        {
                            current_input_mode = INPUT_MODE_NORMAL;
                        }
                    }
                }

                // Input (normal mode)
                if (current_input_mode == INPUT_MODE_NORMAL && !click_on_minimap)
                {
                    // Building selection: check buildings before units
                    if (input->mouse_left_pressed)
                    {
                        float world_x = input->mouse_x - cam_x;
                        float world_y = input->mouse_y - cam_y;
                        uint32_t cx = (uint32_t)(world_x / CELL_W);
                        uint32_t cy = (uint32_t)(world_y / CELL_H);

                        // Deselect all buildings
                        for (uint32_t i = 0; i < buildings.count; i++)
                            buildings.buildings[i].selected = false;

                        int32_t bldg = building_find_at(&buildings, building_types, cx, cy);
                        if (bldg >= 0 && buildings.buildings[bldg].team == 0)
                        {
                            buildings.buildings[bldg].selected = true;
                            selected_building = bldg;
                            // Deselect all units
                            for (uint32_t i = 0; i < units.count; i++)
                                units.units[i].selected = false;
                        }
                        else
                        {
                            selected_building = -1;
                            handle_selection(&units, input, cam_x, cam_y);
                            if (input->mouse_left_released && snd_select != UINT32_MAX)
                            {
                                for (uint32_t i = 0; i < units.count; ++i)
                                    if (units.units[i].selected) { audio_play(snd_select, 0.5f); break; }
                            }
                        }
                    }
                    else
                    {
                        handle_selection(&units, input, cam_x, cam_y);
                        if (input->mouse_left_released && snd_select != UINT32_MAX)
                        {
                            for (uint32_t i = 0; i < units.count; ++i)
                                if (units.units[i].selected) { audio_play(snd_select, 0.5f); break; }
                        }
                    }

                    // Production hotkeys (when building is selected)
                    if (selected_building >= 0 && selected_building < (int32_t)buildings.count)
                    {
                        building* sb = &buildings.buildings[selected_building];
                        const building_type* sbt = &building_types[sb->type_index];
                        if (input->key_1_pressed && sbt->producible_count > 0)
                            building_start_production(sb, sbt, &banks[0], 0);
                        if (input->key_2_pressed && sbt->producible_count > 1)
                            building_start_production(sb, sbt, &banks[0], 1);
                        if (input->key_3_pressed && sbt->producible_count > 2)
                            building_start_production(sb, sbt, &banks[0], 2);
                        if (input->key_4_pressed && sbt->producible_count > 3)
                            building_start_production(sb, sbt, &banks[0], 3);
                    }
                }

                if (input->mouse_right_pressed && snd_move != UINT32_MAX)
                {
                    for (uint32_t i = 0; i < units.count; ++i)
                        if (units.units[i].selected && units.units[i].team == 0) { audio_play(snd_move, 0.5f); break; }
                }
                if (current_input_mode == INPUT_MODE_NORMAL)
                    handle_move_command(&units, &map, input, cam_x, cam_y);
            }

            // Update (always runs, even with console open)
            combat_sounds csounds = {.snd_attack = snd_attack, .snd_die = snd_die};
            units_update(&units, &map, sim_dt, &csounds);
            buildings_update(&buildings, building_types, banks, &units,
                             unit_types, num_unit_types, &map, &mines, sim_dt);
            mines_update(&mines, banks, sim_dt);
        }

        // Render
        if (!gfx_begin_frame())
            continue;

        if (state == GAME_STATE_PLAYING || state == GAME_STATE_PAUSED)
        {
            // Terrain (viewport-culled)
            int start_cx = (int)(-cam_x / CELL_W) - 1;
            int start_cy = (int)(-cam_y / CELL_H) - 1;
            if (start_cx < 0) start_cx = 0;
            if (start_cy < 0) start_cy = 0;
            int end_cx = start_cx + (int)(screen_w / CELL_W) + 3;
            int end_cy = start_cy + (int)(screen_h / CELL_H) + 3;
            if (end_cx > (int)map.width) end_cx = (int)map.width;
            if (end_cy > (int)map.height) end_cy = (int)map.height;
            for (int cy = start_cy; cy < end_cy; cy++)
                for (int cx = start_cx; cx < end_cx; cx++)
                {
                    uint8_t terrain = map_get(&map, (uint32_t)cx, (uint32_t)cy).terrain;
                    if (terrain >= TERRAIN_COUNT) terrain = TERRAIN_VOID;
                    gfx_draw_sprite((float)(cx * CELL_W), (float)(cy * CELL_H),
                                    (float)CELL_W, (float)CELL_H,
                                    terrain_textures[terrain], 0xFFFFFFFF);
                }

            // Mines
            mines_draw(&mines, white_tex);

            // Buildings
            buildings_draw(&buildings, building_types, white_tex);

            // Units + health bars
            units_draw(&units, white_tex);

            // Building placement ghost
            if (current_input_mode == INPUT_MODE_PLACING)
            {
                float world_x = input->mouse_x - cam_x;
                float world_y = input->mouse_y - cam_y;
                uint32_t pcx = (uint32_t)(world_x / CELL_W);
                uint32_t pcy = (uint32_t)(world_y / CELL_H);
                const building_type* pbt = &building_types[placing_type_index];
                float px = (float)(pcx * CELL_W);
                float py = (float)(pcy * CELL_H);
                float pw = (float)(pbt->footprint_w * CELL_W);
                float ph = (float)(pbt->footprint_h * CELL_H);
                bool valid = map_area_clear(&map, pcx, pcy, pbt->footprint_w, pbt->footprint_h, MOVE_LAND)
                             && resource_can_afford(&banks[0], pbt->cost);
                uint32_t ghost_color = valid ? 0x8000FF00 : 0x80FF0000;
                gfx_draw_sprite(px, py, pw, ph, white_tex, ghost_color);
            }

            // Selection box overlay
            draw_selection_box(input, white_tex);

            // Minimap
            if (minimap_tex != UINT32_MAX)
            {
                float mm_w = (float)map.width * minimap_scale;
                float mm_h = (float)map.height * minimap_scale;
                float mm_x = -cam_x + (float)screen_w - mm_w - minimap_margin;
                float mm_y = -cam_y + (float)screen_h - mm_h - minimap_margin;

                gfx_draw_sprite(mm_x, mm_y, mm_w, mm_h, minimap_tex, 0xFFFFFFFF);

                float vp_x = mm_x + (-cam_x / (float)(map.width * CELL_W)) * mm_w;
                float vp_y = mm_y + (-cam_y / (float)(map.height * CELL_H)) * mm_h;
                float vp_w = ((float)screen_w / (float)(map.width * CELL_W)) * mm_w;
                float vp_h = ((float)screen_h / (float)(map.height * CELL_H)) * mm_h;
                float t = 1.0f;
                gfx_draw_sprite(vp_x, vp_y, vp_w, t, white_tex, 0xFFFFFFFF);
                gfx_draw_sprite(vp_x, vp_y + vp_h - t, vp_w, t, white_tex, 0xFFFFFFFF);
                gfx_draw_sprite(vp_x, vp_y, t, vp_h, white_tex, 0xFFFFFFFF);
                gfx_draw_sprite(vp_x + vp_w - t, vp_y, t, vp_h, white_tex, 0xFFFFFFFF);

                for (uint32_t i = 0; i < units.count; ++i)
                {
                    const unit* u = &units.units[i];
                    if (u->state == UNIT_STATE_DYING) continue;
                    float dot_x = mm_x + (u->x / (float)(map.width * CELL_W)) * mm_w;
                    float dot_y = mm_y + (u->y / (float)(map.height * CELL_H)) * mm_h;
                    uint32_t dot_color = (u->team == 0) ? 0xFF00FF00 : 0xFF0000FF;
                    gfx_draw_sprite(dot_x - 1.0f, dot_y - 1.0f, 3.0f, 3.0f, white_tex, dot_color);
                }
            }

            // HUD overlay
            hud_draw(&units, &banks[0], dt, screen_w, screen_h);
        }

        // Dev console overlay
        con_draw(dt, screen_w, screen_h);

        // Menu overlay (main menu or pause)
        if (state != GAME_STATE_PLAYING)
            menu_draw(state, screen_w, screen_h);

        gfx_end_frame();
    }

done:
    audio_shutdown();
    map_free(&map);
    free(unit_defs);
    gfx_shutdown();
    return 0;
}
