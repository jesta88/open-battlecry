#include "renderer.h"
#include "graphics.h"
#include "../common/log.h"
#include <assert.h>

enum
{
    MAX_SPRITES = 1024 * 8,
    MAX_GLYPHS = 2048
};

//static u32 glyph_count;
//static SDL_Texture* glyph_textures[MAX_GLYPHS];
//static SDL_Rect glyph_frame_rects[MAX_GLYPHS];
//static SDL_Rect glyph_screen_rects[MAX_GLYPHS];
//

static wb_renderer_state renderer_state;

void wb_renderer_init(void)
{
    // TODO: Sprite pool
}

void wb_renderer_quit(void)
{

}

void wb_renderer_draw(void)
{
    /*for (int i = 0; i < sprite_count; i++)
    {
        texture_t texture = sprite_textures[i];
        SDL_RenderCopy(sdl_renderer, textures[texture.index], &sprite_frame_rects[i], &sprite_screen_rects[i]);
    }

    for (int i = 0; i < glyph_count; i++)
    {
        texture_t texture = glyph_textures[i];
        SDL_RenderCopy(sdl_renderer, textures[texture.index], &glyph_frame_rects[i], &glyph_screen_rects[i]);
    }*/

    /*if (bits8_is_set(mouse_pressed_bitset, WB_MOUSE_BUTTON_LEFT))
    {
        render_selection_box = false;
        SDL_GetMouseState(&selection_box_anchor_x, &selection_box_anchor_y);
        update_selection_box = true;
    }
    if (bits8_is_set(mouse_released_bitset, WB_MOUSE_BUTTON_LEFT))
    {
        update_selection_box = false;
        render_selection_box = false;
    }
    if (update_selection_box && bits8_is_set(mouse_bitset, WB_MOUSE_BUTTON_LEFT))
    {
        s32 mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
    
        if (mouse_x < selection_box_anchor_x)
        {
            selection_box_rect.x = mouse_x;
            selection_box_rect.w = selection_box_anchor_x - mouse_x;
        }
        else
        {
            selection_box_rect.x = selection_box_anchor_x;
            selection_box_rect.w = mouse_x - selection_box_anchor_x;
        }
    
        if (mouse_y < selection_box_anchor_y)
        {
            selection_box_rect.y = mouse_y;
            selection_box_rect.h = selection_box_anchor_y - mouse_y;
        }
        else
        {
            selection_box_rect.y = selection_box_anchor_y;
            selection_box_rect.h = mouse_y - selection_box_anchor_y;
        }
    
        if (selection_box_rect.w > 2 || selection_box_rect.h > 2)
            render_selection_box = true;
        else
            render_selection_box = false;
    }

    if (render_selection_box)
    {
        SDL_SetRenderDrawColor(sdl_renderer, 0, 0xFF, 0, 0xFF);
        SDL_RenderDrawRect(sdl_renderer, &selection_box_rect);
        SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0, 0, 0xFF);
        SDL_RenderDrawPoint(sdl_renderer, 100, 100);
    }*/
}

//texture_t renderer_add_sprite(image_t* image)
//{
//    SDL_Rect* frame_rect = &sprite_frame_rects[sprite_index];
//    if (SDL_QueryTexture(*texture, NULL, NULL, &frame_rect->w, &frame_rect->h) < 0)
//    {
//        wb_log_error("%s", SDL_GetError());
//        return (texture_t) { UINT32_MAX };
//    }
//
//    frame_rect->w /= 2;
//    frame_rect->h /= 8;
//
//    SDL_Rect* world_rect = &sprite_screen_rects[sprite_index];
//    world_rect->x = 40;
//    world_rect->y = 80;
//    world_rect->w = frame_rect->w;
//    world_rect->h = frame_rect->h;
//
//    return (texture_t) { sprite_index };
//}
//
//texture_t renderer_create_texture(image_t* image)
//{
//    if (texture_count >= MAX_TEXTURES)
//    {
//        wb_log_error("%s", "Max textures reached.");
//    }
//
//    if (image == NULL)
//    {
//        wb_log_error("%s", "Image is null.");
//    }
//
//    u32 texture_index = texture_count++;
//    SDL_Texture** texture = &textures[texture_index];
//
//    *texture = SDL_CreateTextureFromSurface(sdl_renderer, image->sdl_surface);
//    image_free(image);
//
//    if (!(*texture))
//    {
//        wb_log_error("%s", SDL_GetError());
//    }
//
//    return (texture_t) { texture_index };
//}
//
//void renderer_destroy_texture(texture_t texture)
//{
//    /*if (texture.index == UINT16_MAX)
//    {
//        wb_log_error("%s", "Invalid texture.");
//        return;
//    }
//
//    SDL_DestroyTexture(textures[texture.index]);
//    textures[texture.index] = NULL;
//    texture_count--;*/
//    // TODO: Compact array
//}
//
//void renderer_add_text(font_t* font, s16 x, s16 y, const char* text)
//{
//    if (glyph_count >= MAX_GLYPHS)
//    {
//        wb_log_error("%s", "Max glyphs reached.");
//        return;
//    }
//
//    int position_x = x;
//    int position_y = y;
//
//    int text_length = (int) strlen(text);
//    for (int i = 0; i < text_length; i++)
//    {
//        char c = text[i];
//
//        if (c == ' ')
//        {
//            position_x += font->glyphs['j'].advance;
//            continue;
//        }
//        else if (c == '\t')
//        {
//            position_x += font->glyphs['j'].advance * 4;
//            continue;
//        }
//        else if (c == '\n')
//        {
//            position_x = x;
//            position_y += font->glyphs['j'].height + 2;
//            continue;
//        }
//
//        glyph_t* glyph = &font->glyphs[c];
//
//        SDL_Rect* frame_rect = &glyph_frame_rects[glyph_count];
//        frame_rect->x = glyph->x;
//        frame_rect->y = glyph->y;
//        frame_rect->w = glyph->width;
//        frame_rect->h = glyph->height;
//
//        SDL_Rect* screen_rect = &glyph_screen_rects[glyph_count];
//        screen_rect->x = position_x + glyph->x_offset;
//        screen_rect->y = position_y + glyph->y_offset;
//        screen_rect->w = glyph->width;
//        screen_rect->h = glyph->height;
//
//        glyph_textures[glyph_count] = (texture_t){ font->texture_index };
//
//        glyph_count++;
//
//        position_x += glyph->advance;
//    }
//}