#include "renderer.h"
#include "window.h"
#include "image.h"
#include "font.h"
#include "text.h"
#include "../base/math.h"
#include "../base/config.h"
#include "../base/log.h"
#include <SDL2/SDL_render.h>
#include <assert.h>

enum
{
    MAX_TEXTURES = 1024,
    MAX_SPRITES = 2048,
    MAX_GLYPHS = 2048
};

struct texture_t
{
    SDL_Texture* texture;
};

static SDL_Renderer* sdl_renderer;
static SDL_Texture* screen_render_target;

static bool update_selection_box;
static bool render_selection_box;
static int32_t selection_box_anchor_x;
static int32_t selection_box_anchor_y;
static SDL_Rect selection_box_rect;

static uint32_t texture_count;
static SDL_Texture* textures[MAX_TEXTURES];

static uint32_t sprite_count;
static texture_t sprite_textures[MAX_SPRITES];
static SDL_Rect sprite_frame_rects[MAX_SPRITES];
static SDL_Rect sprite_screen_rects[MAX_SPRITES];

static uint32_t glyph_count;
static texture_t glyph_textures[MAX_GLYPHS];
static SDL_Rect glyph_frame_rects[MAX_GLYPHS];
static SDL_Rect glyph_screen_rects[MAX_GLYPHS];

void renderer_init(void* window_handle)
{
    if (!window_handle)
    {
        log_error("%s", "Window handle is null.");
        return;
    }

    uint32_t flags = SDL_RENDERER_ACCELERATED;
    if (c_render_vsync->bool_value) flags |= SDL_RENDERER_PRESENTVSYNC;

    sdl_renderer = SDL_CreateRenderer((SDL_Window*) window_handle, -1, flags);
    if (sdl_renderer == NULL)
    {
        log_error("%s", SDL_GetError());
        return;
    }

    uint16_t screen_width, screen_height;
    window_get_size(&screen_width, &screen_height);

    screen_render_target = SDL_CreateTexture(
        sdl_renderer, SDL_PIXELFORMAT_ABGR32,
        SDL_TEXTUREACCESS_TARGET, screen_width, screen_height);

    if (screen_render_target == NULL)
    {
        log_error("%s", SDL_GetError());
    }
}

void renderer_quit(void)
{
    SDL_DestroyTexture(screen_render_target);
    SDL_DestroyRenderer(sdl_renderer);
    sdl_renderer = NULL;
}

void renderer_draw(void)
{
    SDL_SetRenderTarget(sdl_renderer, screen_render_target);
    SDL_SetRenderDrawColor(sdl_renderer, 0xA5, 0xB7, 0xA4, 0xFF);
    SDL_RenderClear(sdl_renderer);

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

    //    if (bits8_is_set(mouse_pressed_bitset, WB_MOUSE_BUTTON_LEFT))
    //    {
    //        render_selection_box = false;
    //        SDL_GetMouseState(&selection_box_anchor_x, &selection_box_anchor_y);
    //        update_selection_box = true;
    //    }
    //    if (bits8_is_set(mouse_released_bitset, WB_MOUSE_BUTTON_LEFT))
    //    {
    //        update_selection_box = false;
    //        render_selection_box = false;
    //    }
    //    if (update_selection_box && bits8_is_set(mouse_bitset, WB_MOUSE_BUTTON_LEFT))
    //    {
    //        int32_t mouse_x, mouse_y;
    //        SDL_GetMouseState(&mouse_x, &mouse_y);
    //
    //        if (mouse_x < selection_box_anchor_x)
    //        {
    //            selection_box_rect.x = mouse_x;
    //            selection_box_rect.w = selection_box_anchor_x - mouse_x;
    //        }
    //        else
    //        {
    //            selection_box_rect.x = selection_box_anchor_x;
    //            selection_box_rect.w = mouse_x - selection_box_anchor_x;
    //        }
    //
    //        if (mouse_y < selection_box_anchor_y)
    //        {
    //            selection_box_rect.y = mouse_y;
    //            selection_box_rect.h = selection_box_anchor_y - mouse_y;
    //        }
    //        else
    //        {
    //            selection_box_rect.y = selection_box_anchor_y;
    //            selection_box_rect.h = mouse_y - selection_box_anchor_y;
    //        }
    //
    //        if (selection_box_rect.w > 2 || selection_box_rect.h > 2)
    //            render_selection_box = true;
    //        else
    //            render_selection_box = false;
    //    }

    if (render_selection_box)
    {
        SDL_SetRenderDrawColor(sdl_renderer, 0, 0xFF, 0, 0xFF);
        SDL_RenderDrawRect(sdl_renderer, &selection_box_rect);
        SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0, 0, 0xFF);
        SDL_RenderDrawPoint(sdl_renderer, 100, 100);
    }

    SDL_SetRenderTarget(sdl_renderer, NULL);

    SDL_RenderCopy(sdl_renderer, screen_render_target, NULL, NULL);
}

void renderer_present(void)
{
    SDL_RenderPresent(sdl_renderer);
}

//texture_t renderer_add_sprite(image_t* image)
//{
//    SDL_Rect* frame_rect = &sprite_frame_rects[sprite_index];
//    if (SDL_QueryTexture(*texture, NULL, NULL, &frame_rect->w, &frame_rect->h) < 0)
//    {
//        log_error("%s", SDL_GetError());
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

texture_t renderer_create_texture(image_t* image)
{
    if (texture_count >= MAX_TEXTURES)
    {
        log_error("%s", "Max textures reached.");
        return (texture_t) { UINT16_MAX };
    }

    if (image == NULL)
    {
        log_error("%s", "Image is null.");
        return (texture_t) { UINT16_MAX };
    }

    uint32_t texture_index = texture_count++;
    SDL_Texture** texture = &textures[texture_index];

    *texture = SDL_CreateTextureFromSurface(sdl_renderer, image->sdl_surface);
    image_free(image);

    if (!(*texture))
    {
        log_error("%s", SDL_GetError());
        return (texture_t) { UINT16_MAX };
    }

    return (texture_t) { texture_index };
}

void renderer_destroy_texture(texture_t texture)
{
    if (texture.index == UINT16_MAX)
    {
        log_error("%s", "Invalid texture.");
        return;
    }

    SDL_DestroyTexture(textures[texture.index]);
    textures[texture.index] = NULL;
    texture_count--;
    // TODO: Compact array
}

void renderer_add_text(font_t* font, int16_t x, int16_t y, const char* text)
{
    if (glyph_count >= MAX_GLYPHS)
    {
        log_error("%s", "Max glyphs reached.");
        return;
    }

    int position_x = x;
    int position_y = y;

    int text_length = (int) strlen(text);
    for (int i = 0; i < text_length; i++)
    {
        char c = text[i];

        if (c == ' ')
        {
            position_x += font->glyphs['j'].advance;
            continue;
        }
        else if (c == '\t')
        {
            position_x += font->glyphs['j'].advance * 4;
            continue;
        }
        else if (c == '\n')
        {
            position_x = x;
            position_y += font->glyphs['j'].height + 2;
            continue;
        }

        glyph_t* glyph = &font->glyphs[c];

        SDL_Rect* frame_rect = &glyph_frame_rects[glyph_count];
        frame_rect->x = glyph->x;
        frame_rect->y = glyph->y;
        frame_rect->w = glyph->width;
        frame_rect->h = glyph->height;

        SDL_Rect* screen_rect = &glyph_screen_rects[glyph_count];
        screen_rect->x = position_x + glyph->x_offset;
        screen_rect->y = position_y + glyph->y_offset;
        screen_rect->w = glyph->width;
        screen_rect->h = glyph->height;

        glyph_textures[glyph_count] = (texture_t){ font->texture_index };

        glyph_count++;

        position_x += glyph->advance;
    }
}
