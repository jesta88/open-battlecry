#include "graphics.h"
#include "../core/log.h"
#include <SDL_video.h>
#include <SDL_render.h>
#include <string.h>
#include <assert.h>

enum
{
    MAX_RENDER_DRIVERS = 8,
    MAX_BUFFERS = 32,
    MAX_SHADERS = 16,
    MAX_TEXTURES = 1024,
    MAX_PIPELINES = 32,

    DEFAULT_WINDOWED_WIDTH = 1280,
    DEFAULT_WINDOWED_HEIGHT = 720,
};

typedef struct
{
    uint8_t size;
    uint8_t next;
    uint8_t* generations;
    uint8_t* free_indices;
} wb_pool8;

typedef struct
{
    uint16_t size;
    uint16_t next;
    uint16_t* generations;
    uint16_t* free_indices;
} wb_pool16;

struct wb_graphics_state
{
    int windowed_width;
    int windowed_height;
    wb_window_mode window_mode;
    SDL_Window* window;

    int driver_index;
    int driver_count;
    SDL_RendererInfo driver_infos[MAX_RENDER_DRIVERS];

    bool vsync;
    SDL_Renderer* renderer;
    SDL_Texture* screen_render_target;

    wb_pool8 buffer_pool;
    wb_pool8 shader_pool;
    wb_pool16 texture_pool;

    wb_buffer buffers[MAX_BUFFERS];
    wb_shader shaders[MAX_SHADERS];
    wb_texture textures[MAX_TEXTURES];
    SDL_Texture* sdl_textures[MAX_TEXTURES];
};

static wb_graphics_state graphics_state;

const char* k_preferred_driver = "direct3d11";

static void _wb_init_pool8(wb_pool8* pool, uint8_t size);
static void _wb_init_pool16(wb_pool16* pool, uint16_t size);
static uint8_t _wb_pool8_get(const wb_pool8* pool);
static uint16_t _wb_pool16_get(const wb_pool16* pool);

void wb_graphics_init(const wb_graphics_desc* graphics_desc)
{
    SDL_VideoInit(NULL);
    
    const int window_position = SDL_WINDOWPOS_UNDEFINED;
    int window_width = DEFAULT_WINDOWED_WIDTH;
    int window_height = DEFAULT_WINDOWED_HEIGHT;
    uint32_t window_flags = SDL_WINDOW_ALLOW_HIGHDPI;

    if (graphics_desc->window_mode == WB_WINDOW_WINDOWED)
    {
        graphics_state.windowed_width = graphics_desc->window_width;
        graphics_state.windowed_height = graphics_desc->window_height;
    }
    else
    {
        if (graphics_desc->window_mode == WB_WINDOW_BORDERLESS)
        {
            window_flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        else
        {
            window_flags |= SDL_WINDOW_FULLSCREEN;
        }

        SDL_DisplayMode display_mode;
        SDL_GetDesktopDisplayMode(0, &display_mode);
        window_width = display_mode.w;
        window_height = display_mode.h;

        graphics_state.windowed_width = DEFAULT_WINDOWED_WIDTH;
        graphics_state.windowed_height = DEFAULT_WINDOWED_HEIGHT;
    }
    
    graphics_state.window_mode = graphics_desc->window_mode;
	graphics_state.window = SDL_CreateWindow(
			graphics_desc->window_name, window_position, window_position,
			window_width, window_height, window_flags);
	if (!graphics_state.window)
    {
        wb_log_error("%s", SDL_GetError());
        return;
    }

    graphics_state.driver_index = -1;
    graphics_state.driver_count = SDL_GetNumRenderDrivers();
    if (graphics_state.driver_count > MAX_RENDER_DRIVERS)
    {
        wb_log_error("Found %i render drivers, max supported is %i.", graphics_state.driver_count, MAX_RENDER_DRIVERS);
    }
    for (int i = 0; i < graphics_state.driver_count; i++)
    {
        SDL_GetRenderDriverInfo(i, &graphics_state.driver_infos[i]);
        if (graphics_state.driver_index == -1 && strcmp(graphics_state.driver_infos[i].name, k_preferred_driver) == 0)
        {
            graphics_state.driver_index = i;
        }

        wb_log_info("Found render driver: [%d] %s", i, graphics_state.driver_infos[i].name);
    }
    if (graphics_state.driver_index == -1)
    {
        graphics_state.driver_index = 0;
    }
    wb_log_info("Using render driver: [%d] %s", graphics_state.driver_index, graphics_state.driver_infos[graphics_state.driver_index].name);

    uint32_t renderer_flags = SDL_RENDERER_ACCELERATED;
    if (graphics_desc->vsync) renderer_flags |= SDL_RENDERER_PRESENTVSYNC;

    graphics_state.vsync = graphics_desc->vsync;
    graphics_state.renderer = SDL_CreateRenderer(graphics_state.window, graphics_state.driver_index, renderer_flags);
    if (!graphics_state.renderer)
    {
        wb_log_error("%s", SDL_GetError());
        return;
    }
}

void wb_graphics_quit(void)
{
    SDL_DestroyRenderer(graphics_state.renderer);
    SDL_DestroyWindow(graphics_state.window);
    SDL_VideoQuit();
}

wb_graphics_state* wb_graphics_get_state(void)
{
    return &graphics_state;
}

wb_buffer wb_create_buffer(const wb_buffer_desc* desc)
{
    return (wb_buffer){0};
}

wb_texture wb_create_texture(const wb_texture_desc* desc)
{
    int index = _wb_pool16_get(&graphics_state.texture_pool);

    return graphics_state.textures[index];
}

static void _wb_init_pool8(wb_pool8* pool, uint8_t size)
{
    pool->size = size + 1;
    pool->next = 0;

}

static void _wb_init_pool16(wb_pool16* pool, uint16_t size)
{
    pool->size = size + 1;
    pool->next = 0;
}

static uint8_t _wb_pool8_get(const wb_pool8* pool)
{
    return 0;
}

static uint16_t _wb_pool16_get(const wb_pool16* pool)
{
    return 0;
}