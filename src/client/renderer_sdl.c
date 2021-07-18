#include "renderer.h"
#include "window.h"
#include "image.h"
#include "../base/math.h"
#include "../base/config.h"
#include "../base/log.h"
#include <SDL2/SDL_render.h>
#include <assert.h>

enum
{
    MAX_SPRITES = 2048
};

static SDL_Renderer* sdl_renderer;
static SDL_Texture* screen_render_target;

static bool update_selection_box;
static bool render_selection_box;
static int32_t selection_box_anchor_x;
static int32_t selection_box_anchor_y;
static SDL_Rect selection_box_rect;

static uint32_t sprite_count;
static SDL_Texture* sprite_textures[MAX_SPRITES];
static SDL_Rect sprite_frame_rects[MAX_SPRITES];
static SDL_Rect sprite_world_rects[MAX_SPRITES];

void renderer_init(void)
{
    uint32_t flags = SDL_RENDERER_ACCELERATED;
    if (c_render_vsync->bool_value) flags |= SDL_RENDERER_PRESENTVSYNC;

    sdl_renderer = SDL_CreateRenderer(window_get_sdl_window(), -1, flags);
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

    for (int i = 0; i < sprite_count; i++)
    {
        SDL_RenderCopy(sdl_renderer, sprite_textures[i], &sprite_frame_rects[i], &sprite_world_rects[i]);
    }

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

sprite_t renderer_add_sprite(image_t* image)
{
    if (sprite_count >= MAX_SPRITES)
    {
        log_error("Max sprite_textures reached.");
        return (sprite_t) { UINT32_MAX };
    }

    if (image == NULL)
    {
        log_error("Image is null.");
        return (sprite_t) { UINT32_MAX };
    }

    uint32_t sprite_index = sprite_count++;
    SDL_Texture** texture = &sprite_textures[sprite_index];

    *texture = SDL_CreateTextureFromSurface(sdl_renderer, image->sdl_surface);
    image_free(image);

    if (!(*texture))
    {
        log_error("%s", SDL_GetError());
        return (sprite_t) { UINT32_MAX };
    }

    SDL_Rect* frame_rect = &sprite_frame_rects[sprite_index];
    if (SDL_QueryTexture(*texture, NULL, NULL, &frame_rect->w, &frame_rect->h) < 0)
    {
        log_error("%s", SDL_GetError());
        return (sprite_t) { UINT32_MAX };
    }

    frame_rect->w /= 2;
    frame_rect->h /= 8;

    SDL_Rect* world_rect = &sprite_world_rects[sprite_index];
    world_rect->x = 40;
    world_rect->y = 80;
    world_rect->w = frame_rect->w;
    world_rect->h = frame_rect->h;

    return (sprite_t) { sprite_index };
}

void renderer_remove_sprite(sprite_t sprite)
{
    if (sprite.index == UINT32_MAX)
    {
        log_error("Invalid sprite.");
        return;
    }

    SDL_DestroyTexture(sprite_textures[sprite.index]);
    sprite_textures[sprite.index] = NULL;
    sprite_count--;
    // TODO: Compact arrays
}
