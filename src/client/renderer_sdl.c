#include "renderer.h"
#include "window.h"
#include "../shared/config.h"
#include "../shared/log.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_image.h>
#include <assert.h>

enum
{
#ifdef DEVELOPMENT
    WB_MAX_RENDERERS = 8,
#else
    WB_MAX_RENDERERS = 1,
#endif
    WB_MAX_ANIMATED_SPRITES = 2048
};

struct WbRenderer
{
    SDL_Renderer* sdl_renderer;
    SDL_Texture* screen_render_target;

    SDL_Texture* animated_sprites[WB_MAX_ANIMATED_SPRITES];

    bool update_selection_box;
    bool render_selection_box;
    int32_t selection_box_anchor_x;
    int32_t selection_box_anchor_y;
    SDL_Rect selection_box_rect;
};

struct WbTexture
{
    SDL_Texture* sdl_texture;
};

static WbRenderer renderers[WB_MAX_RENDERERS];

WbRenderer* wbCreateRenderer(const WbWindow* window)
{
    assert(window != NULL);

    uint8_t renderer_index = UINT8_MAX;
    for (int i = 0; i < WB_MAX_RENDERERS; i++)
    {
        if (renderers[i].sdl_renderer == NULL)
        {
            renderer_index = i;
            break;
        }
    }
    assert(renderer_index != UINT8_MAX);

    WbRenderer* renderer = &renderers[renderer_index];

    uint32_t flags = SDL_RENDERER_ACCELERATED;
    if (c_render_vsync->bool_value) flags |= SDL_RENDERER_PRESENTVSYNC;

    renderer->sdl_renderer = SDL_CreateRenderer(wbSdlWindow(window), -1, flags);
    if (renderer->sdl_renderer == NULL)
    {
        s_log_error("%s", SDL_GetError());
    }

    if (IMG_Init(IMG_INIT_PNG) < 0)
    {
        s_log_error("%s", IMG_GetError());
    }

    uint16_t screen_width, screen_height;
    wbWindowGetSize(window, &screen_width, &screen_height);

    renderer->screen_render_target = SDL_CreateTexture(
        renderer->sdl_renderer, SDL_PIXELFORMAT_ABGR32,
        SDL_TEXTUREACCESS_TARGET, screen_width, screen_height);

    if (renderer->screen_render_target == NULL)
    {
        s_log_error("%s", SDL_GetError());
    }

    return renderer;
}

//
//unit_textures[0] = wbCreateTexture(renderer, "../assets/images/sides/dwarves/ADBX.png");
//
//if (SDL_QueryTexture(unit_textures[0], NULL, NULL, &unit_frame_rects[0].w, &unit_frame_rects[0].h) < 0)
//{
//    s_log_error("%s", SDL_GetError());
//}
//
//unit_frame_rects[0].w /= 2;
//unit_frame_rects[0].h /= 8;
//
//unit_rects[0].x = 40;
//unit_rects[0].y = 80;
//unit_rects[0].w = unit_frame_rects[0].w;
//unit_rects[0].h = unit_frame_rects[0].h;


void wbDestroyRenderer(WbRenderer* renderer)
{
    assert(renderer != NULL);

    SDL_DestroyTexture(renderer->screen_render_target);
    SDL_DestroyRenderer(renderer->sdl_renderer);
    renderer->sdl_renderer = NULL;
}

WbTexture* wbCreateTexture(const WbRenderer* renderer, const char* file_name)
{
    assert(renderer != NULL);

    SDL_Surface* surface = IMG_Load(file_name);
    if (surface == NULL)
    {
        s_log_error("%s", IMG_GetError());
    }

    SDL_Texture* sdl_texture = SDL_CreateTextureFromSurface(renderer->sdl_renderer, surface);
    if (sdl_texture == NULL)
    {
        s_log_error("%s", SDL_GetError());
    }
    SDL_FreeSurface(surface);

    return NULL;
}

void wbRendererDraw(const WbRenderer* renderer)
{
    assert(renderer != NULL);

    SDL_SetRenderTarget(renderer->sdl_renderer, renderer->screen_render_target);
    SDL_SetRenderDrawColor(renderer->sdl_renderer, 0xA5, 0xB7, 0xA4, 0xFF);
    SDL_RenderClear(renderer->sdl_renderer);

    //SDL_RenderCopy(renderer, unit_textures[0], &unit_frame_rects[0], &unit_rects[0]);

//    if (wbIsBitSet8(mouse_pressed_bitset, WB_MOUSE_BUTTON_LEFT))
//    {
//        render_selection_box = false;
//        SDL_GetMouseState(&selection_box_anchor_x, &selection_box_anchor_y);
//        update_selection_box = true;
//    }
//    if (wbIsBitSet8(mouse_released_bitset, WB_MOUSE_BUTTON_LEFT))
//    {
//        update_selection_box = false;
//        render_selection_box = false;
//    }
//    if (update_selection_box && wbIsBitSet8(mouse_bitset, WB_MOUSE_BUTTON_LEFT))
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

    if (renderer->render_selection_box)
    {
        SDL_SetRenderDrawColor(renderer->sdl_renderer, 0, 0xFF, 0, 0xFF);
        SDL_RenderDrawRect(renderer->sdl_renderer, &renderer->selection_box_rect);
        SDL_SetRenderDrawColor(renderer->sdl_renderer, 0xFF, 0, 0, 0xFF);
        SDL_RenderDrawPoint(renderer->sdl_renderer, 100, 100);
    }

    SDL_SetRenderTarget(renderer->sdl_renderer, NULL);

    SDL_RenderCopy(renderer->sdl_renderer, renderer->screen_render_target, NULL, NULL);
}

void wbRendererPresent(const WbRenderer* renderer)
{
    assert(renderer != NULL);

    SDL_RenderPresent(renderer->sdl_renderer);
}
