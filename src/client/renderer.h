#pragma once

#include "../common/types.h"

typedef struct { u16 index; } wb_texture;

typedef enum
{
    WB_RENDER_LAYER_TERRAIN,
    WB_RENDER_LAYER_SPRITES,
    WB_RENDER_LAYER_UI,
    WB_RENDER_LAYER_COUNT
} wb_render_layer;

typedef struct 
{
    u32 sprite_count;
    wb_texture sprite_textures[MAX_SPRITES];
    wb_rect sprite_frame_rects[MAX_SPRITES];
    wb_rect sprite_screen_rects[MAX_SPRITES];

    bool update_selection_box;
    bool render_selection_box;
    s32 selection_box_anchor_x;
    s32 selection_box_anchor_y;
    wb_rect selection_box_rect;
} wb_renderer_state;

typedef struct
{
    u64 texture_id;
    wb_render_layer render_layer;
    bool transparent;
} wb_sprite_desc;

void wb_renderer_init(void);
void wb_renderer_quit(void);
void wb_renderer_draw(void);

wb_texture wb_texture_load_png(const char* path);
void wb_texture_free(wb_texture texture);