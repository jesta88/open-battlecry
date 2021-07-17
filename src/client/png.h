//#pragma once
//
//#include <stdint.h>
//#include <stdbool.h>
//
//enum pixel_format
//{
//    PIXEL_FORMAT_INVALID = 0x00000000,
//
//    PIXEL_FORMAT_A = 0x02000008,
//
//    PIXEL_FORMAT_Y = 0x20000008,
//    PIXEL_FORMAT_Y_16LE = 0x2000000B,
//    PIXEL_FORMAT_Y_16BE = 0x2010000B,
//    PIXEL_FORMAT_YA_NONPREMUL = 0x21000008,
//    PIXEL_FORMAT_YA_PREMUL = 0x22000008,
//
//    PIXEL_FORMAT_YCBCR = 0x40020888,
//    PIXEL_FORMAT_YCBCRA_NONPREMUL = 0x41038888,
//    PIXEL_FORMAT_YCBCRK = 0x50038888,
//
//    PIXEL_FORMAT_YCOCG = 0x60020888,
//    PIXEL_FORMAT_YCOCGA_NONPREMUL = 0x61038888,
//    PIXEL_FORMAT_YCOCGK = 0x70038888,
//
//    PIXEL_FORMAT_INDEXED__BGRA_NONPREMUL = 0x81040008,
//    PIXEL_FORMAT_INDEXED__BGRA_PREMUL = 0x82040008,
//    PIXEL_FORMAT_INDEXED__BGRA_BINARY = 0x83040008,
//
//    PIXEL_FORMAT_BGR_565 = 0x80000565,
//    PIXEL_FORMAT_BGR = 0x80000888,
//    PIXEL_FORMAT_BGRA_NONPREMUL = 0x81008888,
//    PIXEL_FORMAT_BGRA_NONPREMUL_4X16LE = 0x8100BBBB,
//    PIXEL_FORMAT_BGRA_PREMUL = 0x82008888,
//    PIXEL_FORMAT_BGRA_PREMUL_4X16LE = 0x8200BBBB,
//    PIXEL_FORMAT_BGRA_BINARY = 0x83008888,
//    PIXEL_FORMAT_BGRX = 0x90008888,
//
//    PIXEL_FORMAT_RGB = 0xA0000888,
//    PIXEL_FORMAT_RGBA_NONPREMUL = 0xA1008888,
//    PIXEL_FORMAT_RGBA_NONPREMUL_4X16LE = 0xA100BBBB,
//    PIXEL_FORMAT_RGBA_PREMUL = 0xA2008888,
//    PIXEL_FORMAT_RGBA_PREMUL_4X16LE = 0xA200BBBB,
//    PIXEL_FORMAT_RGBA_BINARY = 0xA3008888,
//    PIXEL_FORMAT_RGBX = 0xB0008888,
//
//    PIXEL_FORMAT_CMY = 0xC0020888,
//    PIXEL_FORMAT_CMYK = 0xD0038888,
//
//    PIXEL_FORMAT_NUM_PLANES_MAX = 4,
//};
//
//struct png_decoder;
//
//struct png_empty
//{
//    uint8_t byte;
//};
//
//struct png_vtable
//{
//    const char* name;
//    const void* function_pointers;
//};
//
//#define DECLARE_SLICE(T, name)  \
//    struct name {               \
//        T* ptr;                 \
//        size_t length;          \
//    }
//
//#define DECLARE_TABLE(T, name)  \
//    struct name {               \
//        T* ptr;                 \
//        uint64_t width;         \
//        uint64_t height;        \
//        uint64_t stride;        \
//    }
//
//DECLARE_SLICE(uint8_t, slice_u8);
//
//DECLARE_TABLE(uint8_t, table_u8);
//
//struct io_buffer_meta
//{
//    uint64_t write_index;
//    uint64_t read_index;
//    uint64_t position;
//    bool closed;
//};
//
//struct io_buffer
//{
//    struct slice_u8 data;
//    struct io_buffer_meta meta;
//};
//
//struct pixel_config
//{
//    uint32_t pixel_format;
//    uint32_t pixel_subsampling;
//    uint32_t width;
//    uint32_t height;
//};
//
//struct pixel_buffer
//{
//    struct pixel_config pixel_config;
//    struct table_u8 planes[PIXEL_FORMAT_NUM_PLANES_MAX];
//};
//
//typedef uint64_t (* pixel_swizzler_func)(
//    uint8_t* dst_ptr,
//    size_t dst_length,
//    uint8_t* dst_palette_ptr,
//    size_t dst_palette_length,
//    const uint8_t* src_ptr,
//    size_t src_length);
//
//typedef uint64_t (* pixel_swizzler_transparent_black_func)(
//    uint8_t* dst_ptr,
//    size_t dst_length,
//    uint8_t* dst_palette_ptr,
//    size_t dst_palette_length,
//    uint64_t num_pixels,
//    uint32_t dst_bytes_per_pixel);
//
//struct pixel_swizzler
//{
//    pixel_swizzler_func func;
//    pixel_swizzler_transparent_black_func transparent_black_func;
//    uint32_t dst_bytes_per_pixel;
//    uint32_t src_bytes_per_pixel;
//};
//
//struct image_config
//{
//    struct pixel_config pixel_config;
//    struct
//    {
//        uint64_t first_frame_io_position;
//        bool first_frame_is_opaque;
//    } private;
//};
//
//struct rect_ie_u32
//{
//    uint32_t min_incl_x;
//    uint32_t min_incl_y;
//    uint32_t max_excl_x;
//    uint32_t max_excl_y;
//};
//
//typedef int64_t flicks;
//
//#define FLICKS_PER_SECOND ((uint64_t)705600000)
//#define FLICKS_PER_MILLISECOND ((uint64_t)705600)
//
//struct frame_config
//{
//    struct rect_ie_u32 bounds;
//    flicks duration;
//    uint64_t index;
//    uint64_t io_position;
//    bool opaque_within_bounds;
//    bool overwrite_instead_of_blend;
//    uint32_t background_color;
//};
//
//void png_decoder_init(struct png_decoder* png_decoder, uint64_t png_decoder_size);
//bool png_decode_image_config(struct png_decoder* png_decoder,
//                             struct image_config* dst,
//                             struct io_buffer* src);
//void png_decode_frame_config(struct png_decoder* png_decoder,
//                             struct frame_config* dst,
//                             struct io_buffer* src);
//void png_decode_frame(struct png_decoder* png_decoder,
//                      struct pixel_buffer* dst,
//                      struct io_buffer* src,
//                      uint8_t blend,
//                      struct slice_u8 buffer);
//
//static inline struct table_u8 make_table_u8(uint8_t* ptr,
//                                            uint64_t width,
//                                            uint64_t height,
//                                            uint64_t stride)
//{
//    struct table_u8 table;
//    table.ptr = ptr;
//    table.width = width;
//    table.height = height;
//    table.stride = stride;
//    return table;
//}
//
//void pixel_buffer_set_interleaved(struct pixel_buffer* pixel_buffer,
//                                  const struct pixel_config* pixel_config,
//                                  struct table_u8 primary_memory,
//                                  struct table_u8 palette_memory);
//
