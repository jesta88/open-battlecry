#include "image.h"
#include "../base/file.h"
#include "../base/log.h"
#include "../base/bits.inl"
#define WUFFS_CONFIG__MODULES
#define WUFFS_CONFIG__MODULE__BASE
#define WUFFS_CONFIG__MODULE__CRC32
#define WUFFS_CONFIG__MODULE__ADLER32
#define WUFFS_CONFIG__MODULE__DEFLATE
#define WUFFS_CONFIG__MODULE__ZLIB
#define WUFFS_CONFIG__MODULE__PNG
#define WUFFS_IMPLEMENTATION
#include "wuffs.c"
#include <assert.h>
#include <SDL2/SDL_surface.h>

static wuffs_png__decoder png_decoder;

static bool check_wuffs_status(wuffs_base__status status)
{
    if (wuffs_base__status__is_ok(&status))
    {
        return true;
    }
    if (wuffs_base__status__is_error(&status))
    {
        log_error(wuffs_base__status__message(&status));
        return false;
    }
    log_info(wuffs_base__status__message(&status));
    return true;
}

void image_init_decoders(void)
{
    wuffs_base__status status;

    status = wuffs_png__decoder__initialize(
        &png_decoder,
        sizeof__wuffs_png__decoder(),
        WUFFS_VERSION,
        WUFFS_INITIALIZE__ALREADY_ZEROED);
    if (!check_wuffs_status(status))
    {
        return;
    }

    wuffs_png__decoder__set_quirk_enabled(&png_decoder, WUFFS_BASE__QUIRK_IGNORE_CHECKSUM, true);
}

void image_load(const char* file_name, image_t* image)
{
    // Input buffer
    uint32_t png_bytes_size;
    file_t png_file = file_size(file_name, &png_bytes_size, false);

    // TODO: Use a stack allocator
    uint8_t* png_bytes = malloc(png_bytes_size);
    if (!png_bytes)
    {
        log_error("Failed to allocate memory of size: %i", png_bytes_size);
        return;
    }
    file_read(png_file, &png_bytes_size, png_bytes);

    wuffs_base__io_buffer io_buffer = wuffs_base__ptr_u8__reader(png_bytes, png_bytes_size, true);
    image->size = png_bytes_size;

    wuffs_base__status status;

    // Image config
    wuffs_base__image_config image_config = {0};
    status = wuffs_png__decoder__decode_image_config(&png_decoder, &image_config, &io_buffer);
    if (!check_wuffs_status(status))
    {
        free(png_bytes);
        return;
    }

    uint32_t width = wuffs_base__pixel_config__width(&image_config.pixcfg);
    uint32_t height = wuffs_base__pixel_config__height(&image_config.pixcfg);
    image->width = width;
    image->height = height;

    // Surface
    wuffs_base__pixel_format wuffs_pixel_format = wuffs_base__pixel_config__pixel_format(&image_config.pixcfg);
    uint32_t pixel_format = SDL_PIXELFORMAT_BGRA32;

    uint32_t plane_count = wuffs_base__pixel_format__num_planes(&wuffs_pixel_format);

    if (wuffs_pixel_format.repr == WUFFS_BASE__PIXEL_FORMAT__BGR)
    {
        pixel_format = SDL_PIXELFORMAT_BGR24;
    }

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
        0, (int) width, (int) height, (int) plane_count * 8, pixel_format);
    if (!surface)
    {
        log_error("%s", SDL_GetError());
        free(png_file);
        return;
    }
    image->sdl_surface = surface;
    image->pixel_format = pixel_format;

    // Pixel buffer
    SDL_LockSurface(surface);
    wuffs_base__table_u8 table = wuffs_base__make_table_u8(surface->pixels, surface->w * 4, surface->h, surface->pitch);
    SDL_UnlockSurface(surface);

    wuffs_base__pixel_buffer pixel_buffer;
    status = wuffs_base__pixel_buffer__set_interleaved(&pixel_buffer, &image_config.pixcfg, table, wuffs_base__empty_slice_u8());
    if (!check_wuffs_status(status))
    {
        SDL_FreeSurface(surface);
        free(png_bytes);
        return;
    }

    // Work buffer
    wuffs_base__range_ii_u64 work_buffer_range = wuffs_png__decoder__workbuf_len(&png_decoder);
    size_t work_buffer_length = work_buffer_range.max_incl;

    uint8_t* work_buffer_data = malloc(work_buffer_length);
    if (!work_buffer_data)
    {
        log_error("Failed to allocate memory of size: %i", work_buffer_length);
        return;
    }
    wuffs_base__slice_u8 work_buffer = wuffs_base__make_slice_u8(work_buffer_data, work_buffer_length);

    // Frame
    status = wuffs_png__decoder__decode_frame(
        &png_decoder,
        &pixel_buffer,
        &io_buffer,
        WUFFS_BASE__PIXEL_BLEND__SRC,
        work_buffer,
        NULL);
    if (!check_wuffs_status(status))
    {
        SDL_FreeSurface(surface);
        free(work_buffer_data);
        free(png_bytes);
        return;
    }

    free(png_bytes);
    free(work_buffer_data);
}

void image_free(image_t* image)
{
    if (!image)
    {
        log_error("Trying to free a null image.");
        return;
    }

    if (!image->sdl_surface)
    {
        log_error("Trying to free a null SDL surface.");
        return;
    }

    SDL_FreeSurface(image->sdl_surface);
    image->sdl_surface = NULL;
    image = NULL;
}
