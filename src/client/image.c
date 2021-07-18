#include "image.h"
#include "../base/io.h"
#define WUFFS_CONFIG__MODULES
#define WUFFS_CONFIG__MODULE__BASE
#define WUFFS_CONFIG__MODULE__CRC32
#define WUFFS_CONFIG__MODULE__ADLER32
#define WUFFS_CONFIG__MODULE__DEFLATE
#define WUFFS_CONFIG__MODULE__ZLIB
#define WUFFS_CONFIG__MODULE__PNG
#define WUFFS_IMPLEMENTATION
#include "wuffs.c"
#include "../base/log.h"
#include <stdio.h>
#include <assert.h>

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

static void create_io_buffer_from_file(const char* file_name, wuffs_base__io_buffer* io_buffer)
{
    uint32_t buffer_size;
    file_t file = io_file_size(file_name, &buffer_size, false);

    // TODO: Use a stack allocator 
    uint8_t* buffer = malloc(buffer_size);
    if (!buffer)
    {
        log_error("Failed to allocate memory of size: %i", buffer_size);
        return;
    }
    io_read_file(file, &buffer_size, buffer);

    *io_buffer = wuffs_base__ptr_u8__writer(buffer, buffer_size);

    log_debug("IO buffer length: %i", wuffs_base__io_buffer__writer_length(io_buffer));
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

void image_load_png(const char* file_name, bool transparent, uint8_t flags, image_t* image)
{

    wuffs_base__io_buffer io_buffer;
    create_io_buffer_from_file(file_name, &io_buffer);


    wuffs_base__status status;

    wuffs_base__image_config image_config = { 0 };
    status = wuffs_png__decoder__decode_image_config(&png_decoder, &image_config, &io_buffer);
    if (!check_wuffs_status(status))
    {
        return;
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
        return;
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

    wuffs_base__frame_config frame_config = { 0 };
    status = wuffs_png__decoder__decode_frame(
        &png_decoder,
        &pixel_buffer,
        &io_buffer,
        WUFFS_BASE__PIXEL_BLEND__SRC,
        workbuf,
        NULL);
    if (!check_wuffs_status(status))
    {
        return;
    }
}
