#include "../common/log.h"
#include "../common/pool.h"
#include "graphics.h"
#include <SDL_render.h>
#include <SDL_video.h>
#include <assert.h>
#include <string.h>

enum
{
	MAX_RENDER_DRIVERS = 8,
	MAX_BUFFERS = 32,
	MAX_SHADERS = 16,
	MAX_TEXTURES = 1024,
	MAX_PIPELINES = 32,
	MAX_RENDER_TARGETS = 32,

	DEFAULT_WINDOWED_WIDTH = 1280,
	DEFAULT_WINDOWED_HEIGHT = 720,
};

typedef struct {
	int windowed_width;
	int windowed_height;
	wb_window_mode window_mode;
	SDL_Window* window;

	int driver_index;
	int driver_count;
	SDL_RendererInfo driver_infos[MAX_RENDER_DRIVERS];

	bool vsync;
	SDL_Renderer* renderer;
	
	u32 texture_count;
	wb_texture texture_handles[MAX_TEXTURES];
	SDL_Texture* textures[MAX_TEXTURES];

	u32 render_target_count;
	wb_render_target render_target_handles[MAX_RENDER_TARGETS];
	SDL_Texture* render_targets[MAX_RENDER_TARGETS];
} wb_graphics_state;

static wb_graphics_state graphics_state;

#ifdef _WIN32
const char* k_preferred_driver = "direct3d11";
#endif

void wb_graphics_init(const wb_graphics_desc* graphics_desc)
{
	int result = SDL_VideoInit(NULL);
	if (result != 0)
	{
		wb_log_error("%s", SDL_GetError());
		assert(0);
	}

	const int window_position = SDL_WINDOWPOS_UNDEFINED;
	int window_width = DEFAULT_WINDOWED_WIDTH;
	int window_height = DEFAULT_WINDOWED_HEIGHT;
	u32 window_flags = SDL_WINDOW_ALLOW_HIGHDPI;

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
		assert(0);
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

	u32 renderer_flags = SDL_RENDERER_ACCELERATED;
	if (graphics_desc->vsync) renderer_flags |= SDL_RENDERER_PRESENTVSYNC;

	graphics_state.vsync = graphics_desc->vsync;
	graphics_state.renderer = SDL_CreateRenderer(graphics_state.window, graphics_state.driver_index, renderer_flags);
	if (!graphics_state.renderer)
	{
		wb_log_error("%s", SDL_GetError());
		assert(0);
	}
}

void wb_graphics_quit(void)
{
	SDL_DestroyRenderer(graphics_state.renderer);
	SDL_DestroyWindow(graphics_state.window);
	SDL_VideoQuit();
}

void wb_graphics_clear(void)
{
	SDL_RenderClear(graphics_state.renderer);
}

void wb_graphics_present(void)
{
	SDL_RenderPresent(graphics_state.renderer);
}

void wb_graphics_set_clear_color(u8 r, u8 g, u8 b, u8 a)
{
	SDL_SetRenderDrawColor(graphics_state.renderer, r, g, b, a);
}

void wb_graphics_set_render_target(wb_render_target render_target)
{
	SDL_SetRenderTarget(graphics_state.renderer, graphics_state.render_targets[render_target.id]);
}

void wb_graphics_set_null_render_target(void)
{
	SDL_SetRenderTarget(graphics_state.renderer, NULL);
}

wb_buffer wb_graphics_create_buffer(const wb_buffer_desc* desc)
{
	return (wb_buffer){0};
}

wb_texture wb_graphics_create_texture(const wb_texture_desc* desc)
{
	return (wb_texture){0};
}

wb_render_target wb_graphics_create_render_target(const wb_render_target_desc* render_target_desc)
{
	int index = graphics_state.render_target_count++;
	graphics_state.render_targets[index] = SDL_CreateTexture(graphics_state.renderer,
															SDL_PIXELFORMAT_BGRA8888,
															SDL_TEXTUREACCESS_TARGET,
															render_target_desc->width,
															render_target_desc->height);
	return (wb_render_target) { index };
}
