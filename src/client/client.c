#include "client.h"
#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "image.h"
#include "font.h"
#include "../base/config.h"
#include <SDL2/SDL.h>

config_t* c_quit;

config_t* c_window_width;
config_t* c_window_height;
config_t* c_window_fullscreen;
config_t* c_window_borderless;

config_t* c_render_vsync;
config_t* c_render_scale;

config_t* c_camera_zoom;
config_t* c_camera_speed;

config_t* c_audio_master_volume;
config_t* c_audio_music_volume;
config_t* c_audio_sfx_volume;
config_t* c_audio_voice_volume;
config_t* c_audio_ambient_volume;

static SDL_Window* window;

static const char* config_file_name = "config.txt";

static void load_configs(void);
static void create_window(void);

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    load_configs();
    create_window();
    renderer_init(window);
    image_init_decoders();

    font_t arial_32;
    font_load("../../assets/fonts/consolas_20.fnt", &arial_32);

    renderer_add_text(&arial_32, 40, 80, "Yo whatsup!");

    //image_t test_image = {0};
    //image_load("../assets/images/sides/dwarves/ADBX.png", IMAGE_LOAD_TRANSPARENT, &test_image);

    //texture_t test_sprite = renderer_add_sprite(&test_image);

    //assert(test_image.sdl_surface == NULL);

    char title[128];
    uint64_t last_tick = SDL_GetPerformanceCounter();

    while (!c_quit->bool_value)
    {
        SDL_PumpEvents();

        // Window Events
        {
            SDL_Event events[8];
            const int count = SDL_PeepEvents(events, 8, SDL_GETEVENT, SDL_QUIT, SDL_SYSWMEVENT);

            for (int i = 0; i < count; i++)
            {
                SDL_Event* event = &events[i];
                if (event->type == SDL_QUIT)
                {
                    c_quit->bool_value = true;
                }
            }
        }

        c_handle_input_events();

        // Update

        // Render
        renderer_draw();
        renderer_present();

        // Stats
        uint64_t dt = 0;
        uint64_t tick = SDL_GetPerformanceCounter();
        dt = tick - last_tick;
        last_tick = tick;
        double ms = (dt * 1000.0) / SDL_GetPerformanceFrequency();
        sprintf(title, "Open Battlecry - %.2f ms", ms);

        SDL_SetWindowTitle(window, title);
    }

    config_save(config_file_name);

    //renderer_destroy_texture(test_sprite);

    renderer_quit();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

static void load_configs(void)
{
    config_load(config_file_name);

    c_quit = config_get_bool("c_quit", false, 0);

    c_window_width = config_get_int("c_window_width", 1280, CONFIG_SAVE);
    c_window_height = config_get_int("c_window_height", 720, CONFIG_SAVE);
    c_window_fullscreen = config_get_bool("c_window_fullscreen", false, CONFIG_SAVE);
    c_window_borderless = config_get_bool("c_window_borderless", false, CONFIG_SAVE);

    c_render_vsync = config_get_bool("c_render_vsync", false, CONFIG_SAVE);
    c_render_scale = config_get_float("c_render_scale", 1.0f, CONFIG_SAVE);

    c_audio_master_volume = config_get_float("c_audio_master_volume", 1.0f, CONFIG_SAVE);
    c_audio_music_volume = config_get_float("c_audio_music_volume", 1.0f, CONFIG_SAVE);
    c_audio_sfx_volume = config_get_float("c_audio_sfx_volume", 1.0f, CONFIG_SAVE);
    c_audio_ambient_volume = config_get_float("c_audio_ambient_volume", 1.0f, CONFIG_SAVE);
    c_audio_voice_volume = config_get_float("c_audio_voice_volume", 1.0f, CONFIG_SAVE);
}

static void create_window(void)
{
    int32_t position = SDL_WINDOWPOS_UNDEFINED;

    bool borderless = c_window_borderless->bool_value;
    bool fullscreen = c_window_fullscreen->bool_value;

    uint32_t flags = SDL_WINDOW_ALLOW_HIGHDPI;
    if (fullscreen && borderless) flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN_DESKTOP;
    else if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

    window = SDL_CreateWindow(
        "Open Battlecry", position, position,
        c_window_width->int_value, c_window_height->int_value, flags);

    if (window == NULL)
    {
        log_error("%s", SDL_GetError());
    }
}