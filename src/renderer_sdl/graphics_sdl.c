//#ifndef SERVER
//#include "../../include/carbon/graphics.h"
//#include "../../include/carbon/log.h"
//#include "../../include/carbon/config.h"
//#include "../../include/carbon/memory.h"
//#include <SDL2/SDL_render.h>
//#include <SDL2/SDL_assert.h>
//
//enum
//{
//    WS_MAX_DRIVERS = 4,
//    WS_MAX_BUFFERS = 32,
//    WS_MAX_SHADERS = 16,
//    WS_MAX_TEXTURES = 1024,
//    WS_MAX_PIPELINES = 32
//};
//
//typedef struct
//{
//    uint8_t size;
//    uint8_t next;
//    uint8_t* generations;
//    uint8_t* free_indices;
//} ws_pool8;
//
//typedef struct
//{
//    uint16_t size;
//    uint16_t next;
//    uint16_t* generations;
//    uint16_t* free_indices;
//} ws_pool16;
//
//// ============================================================================
//// Variables
//// ============================================================================
//static SDL_RendererInfo driver_infos[WS_MAX_DRIVERS];
//static uint32_t driver_count;
//
//static SDL_Renderer* renderer;
//
//static ws_pool8 buffer_pool;
//static ws_pool8 shader_pool;
//static ws_pool16 texture_pool;
//
//static ws_buffer buffers[WS_MAX_BUFFERS];
//static ws_shader shaders[WS_MAX_SHADERS];
//static ws_texture textures[WS_MAX_TEXTURES];
//
//// ============================================================================
//// Forward declarations
//// ============================================================================
//static void _ws_find_render_drivers(void);
//static void _ws_init_pool8(ws_pool8* pool, uint8_t size);
//static void _ws_init_pool16(ws_pool16* pool, uint16_t size);
//static uint8_t _ws_pool8_get(const ws_pool8* pool);
//static uint16_t _ws_pool16_get(const ws_pool16* pool);
//
//// ============================================================================
//// Public API implementation
//// ============================================================================
//void ws_init_graphics(void* window_handle)
//{
//    _ws_find_render_drivers();
//
//    if (!window_handle)
//    {
//        log_error("%s", "Window handle is null.");
//        return;
//    }
//
//    uint32_t flags = SDL_RENDERER_ACCELERATED;
//    if (c_render_vsync->bool_value) flags |= SDL_RENDERER_PRESENTVSYNC;
//
//    renderer = SDL_CreateRenderer((SDL_Window*)window_handle, -1, flags);
//    if (!renderer)
//    {
//        log_error("%s", SDL_GetError());
//        return;
//    }
//}
//
//void ws_quit_graphics(void)
//{
//    SDL_DestroyRenderer(renderer);
//}
//
//ws_buffer ws_create_buffer(const ws_buffer_desc* desc)
//{
//    return (ws_buffer){0};
//}
//
//ws_texture ws_create_texture(const ws_texture_desc* desc)
//{
//    SDL_assert(renderer);
//    SDL_assert(desc);
//
//    int index = _ws_pool16_get(&texture_pool);
//
//    return textures[index];
//}
//
//// ============================================================================
//// Private implementation
//// ============================================================================
//static void _ws_find_render_drivers(void)
//{
//    driver_count = SDL_GetNumRenderDrivers();
//    if (driver_count > WS_MAX_DRIVERS)
//    {
//        log_error("Found %i render drivers, max supported is %i.", driver_count, WS_MAX_DRIVERS);
//    }
//    for (int i = 0; i < driver_count; i++)
//    {
//        SDL_GetRenderDriverInfo(i, &driver_infos[i]);
//        log_info("Found render driver: %s", driver_infos[i].name);
//    }
//}
//
//static void _ws_init_pool8(ws_pool8* pool, uint8_t size)
//{
//    SDL_assert(pool);
//    SDL_assert(size > 1);
//    SDL_assert(size < (UINT8_MAX - 1));
//    pool->size = size + 1;
//    pool->next = 0;
//
//}
//
//static void _ws_init_pool16(ws_pool16* pool, uint16_t size)
//{
//    SDL_assert(pool);
//    SDL_assert(size > 1);
//    SDL_assert(size < (UINT16_MAX - 1));
//    pool->size = size + 1;
//    pool->next = 0;
//}
//
//static uint8_t _ws_pool8_get(const ws_pool8* pool)
//{
//    return 0;
//}
//
//static uint16_t _ws_pool16_get(const ws_pool16* pool)
//{
//    return 0;
//}
//#endif