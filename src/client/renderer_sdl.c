#include "renderer.h"
#include "window.h"
#include "../base/config.h"
#include "../base/log.h"
#include <SDL2/SDL_render.h>
#include <assert.h>
#define WUFFS_CONFIG__MODULES
#define WUFFS_CONFIG__MODULE__BASE
#define WUFFS_CONFIG__MODULE__CRC32
#define WUFFS_CONFIG__MODULE__ADLER32
#define WUFFS_CONFIG__MODULE__DEFLATE
#define WUFFS_CONFIG__MODULE__ZLIB
#define WUFFS_CONFIG__MODULE__PNG
#define WUFFS_IMPLEMENTATION
#include "wuffs.c"

enum
{
#ifdef DEVELOPMENT
    WB_MAX_RENDERERS = 8,
#else
    WB_MAX_RENDERERS = 1,
#endif
    WB_MAX_ANIMATED_SPRITES = 2048
};

struct renderer
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

struct texture
{
    SDL_Texture* sdl_texture;
};

static struct renderer renderers[WB_MAX_RENDERERS];

struct renderer* wbCreateRenderer(const struct window* window)
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

    struct renderer* renderer = &renderers[renderer_index];

    uint32_t flags = SDL_RENDERER_ACCELERATED;
    if (c_render_vsync->bool_value) flags |= SDL_RENDERER_PRESENTVSYNC;

    renderer->sdl_renderer = SDL_CreateRenderer(wbSdlWindow(window), -1, flags);
    if (renderer->sdl_renderer == NULL)
    {
        log_error("%s", SDL_GetError());
    }

    uint16_t screen_width, screen_height;
    wbWindowGetSize(window, &screen_width, &screen_height);

    renderer->screen_render_target = SDL_CreateTexture(
        renderer->sdl_renderer, SDL_PIXELFORMAT_ABGR32,
        SDL_TEXTUREACCESS_TARGET, screen_width, screen_height);

    if (renderer->screen_render_target == NULL)
    {
        log_error("%s", SDL_GetError());
    }

    return renderer;
}

//
//unit_textures[0] = wbCreateTexture(renderer, "../assets/images/sides/dwarves/ADBX.png");
//
//if (SDL_QueryTexture(unit_textures[0], NULL, NULL, &unit_frame_rects[0].w, &unit_frame_rects[0].h) < 0)
//{
//    log_error("%s", SDL_GetError());
//}
//
//unit_frame_rects[0].w /= 2;
//unit_frame_rects[0].h /= 8;
//
//unit_rects[0].x = 40;
//unit_rects[0].y = 80;
//unit_rects[0].w = unit_frame_rects[0].w;
//unit_rects[0].h = unit_frame_rects[0].h;


void wbDestroyRenderer(struct renderer* renderer)
{
    assert(renderer != NULL);

    SDL_DestroyTexture(renderer->screen_render_target);
    SDL_DestroyRenderer(renderer->sdl_renderer);
    renderer->sdl_renderer = NULL;
}

struct texture* wbCreateTexture(const struct renderer* renderer, const char* file_name, bool transparent)
{
    struct texture* texture = NULL;

    FILE* file = fopen(file_name, "rb");
    if (!file)
    {
        log_error("Failed to open file: %s", file_name);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t* buffer = malloc(file_size);
    if (!buffer)
    {
        log_error("Failed to allocate memory of size: %ull", file_size);
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read == 0)
    {
        log_error("Failed to read bytes from file: %s", file_name);
        goto bail;
    }

    wuffs_base__io_buffer io_buffer = wuffs_base__ptr_u8__reader(buffer, bytes_read, true);

    wuffs_base__status status;
    wuffs_png__decoder png_decoder = {0};

    status = wuffs_png__decoder__initialize(
        &png_decoder,
        sizeof__wuffs_png__decoder(),
        WUFFS_VERSION,
        WUFFS_INITIALIZE__ALREADY_ZEROED);
    if (!wuffs_base__status__is_ok(&status))
    {
        log_error(wuffs_base__status__message(&status));
        goto bail;
    }

    wuffs_png__decoder__set_quirk_enabled(&png_decoder, WUFFS_BASE__QUIRK_IGNORE_CHECKSUM, true);

    wuffs_base__image_config image_config = {0};
    status = wuffs_png__decoder__decode_image_config(&png_decoder, &image_config, &io_buffer);
    if (!wuffs_base__status__is_ok(&status))
    {
        log_error(wuffs_base__status__message(&status));
        goto bail;
    }

    uint32_t width = wuffs_base__pixel_config__width(&image_config.pixcfg);
    uint32_t height = wuffs_base__pixel_config__height(&image_config.pixcfg);
    wuffs_base__pixel_format pixel_format = wuffs_base__make_pixel_format(
        transparent
        ? WUFFS_BASE__PIXEL_FORMAT__RGBA_NONPREMUL
        : WUFFS_BASE__PIXEL_FORMAT__RGB);

    ///wuffs_base__table_u8 table = wuffs_base__make_table_u8(buffer, )

    wuffs_base__range_ii_u64 workbuf_range = wuffs_png__decoder__workbuf_len(&png_decoder);
    size_t workbuf_length = workbuf_range.max_incl;

    uint8_t* ptr = malloc(workbuf_length);
    if (!ptr)
    {
        log_error("Failed to read bytes from file: %s", file_name);
        goto bail;
    }
    wuffs_base__slice_u8 workbuf = wuffs_base__make_slice_u8(ptr, workbuf_length);
    free(ptr);

    wuffs_base__pixel_buffer pixel_buffer;

//    status = wuffs_base__pixel_buffer__set_from_table(&pixel_buffer, &image_config.pixcfg,);
//    if (!wuffs_base__status__is_ok(&status))
//    {
//        log_error(wuffs_base__status__message(&status));
//        goto bail;
//    }

    //    wuffs_base__pixel_buffer__set_interleaved(&pixel_buffer, &image_config.pixcfg,
    //                                              wuffs_base__make_table_u8((uint8_t)(m_surface->pixels),
    //                                                                        width * 4, height,
    //                                                                        m_surface->pitch),
    //                                                                        wuffs_base__empty_slice_u8())

    wuffs_base__frame_config frame_config = {0};
    status = wuffs_png__decoder__decode_frame(
        &png_decoder,
        &pixel_buffer,
        &io_buffer,
        WUFFS_BASE__PIXEL_BLEND__SRC,
        workbuf,
        NULL);
    if (!wuffs_base__status__is_ok(&status))
    {
        log_error(wuffs_base__status__message(&status));
        goto bail;
    }

    texture = malloc(sizeof(*texture));
    if (!texture)
    {
        log_error("Failed to allocate memory of size: %ull", sizeof(struct texture));
        goto bail;
    }

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(workbuf.ptr, (int) width, (int) height,
                                                            32, 4 * (int) width, SDL_PIXELFORMAT_RGBA32);
    if (!surface)
    {
        log_error("%s", SDL_GetError());
        goto bail;
    }

    SDL_Texture* sdl_texture = SDL_CreateTextureFromSurface(renderers[0].sdl_renderer, surface);
    if (!sdl_texture)
    {
        log_error("%s", SDL_GetError());
        goto bail;
    }

    SDL_FreeSurface(surface);

    bail:
    free(buffer);
    fclose(file);

    return texture;
}

void wbRendererDraw(const struct renderer* renderer)
{
    assert(renderer != NULL);

    SDL_SetRenderTarget(renderer->sdl_renderer, renderer->screen_render_target);
    SDL_SetRenderDrawColor(renderer->sdl_renderer, 0xA5, 0xB7, 0xA4, 0xFF);
    SDL_RenderClear(renderer->sdl_renderer);

    //SDL_RenderCopy(renderer, unit_textures[0], &unit_frame_rects[0], &unit_rects[0]);

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

void wbRendererPresent(const struct renderer* renderer)
{
    assert(renderer != NULL);

    SDL_RenderPresent(renderer->sdl_renderer);
}
