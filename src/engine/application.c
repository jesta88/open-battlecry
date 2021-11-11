#include "private.h"
#include <engine/application.h>
#include <engine/config.h>
#include <engine/log.h>
#include <SDL2/SDL.h>

static struct app_desc app_desc;

static SDL_Window* window;

struct config* c_quit;
struct config* c_window_width;
struct config* c_window_height;
struct config* c_window_fullscreen;
struct config* c_window_borderless;

static const char* config_file_name = "config.txt";

static void load_configs(void);
static void create_window(void);
static void handle_window_events(void);

int main(int argc, char* argv[])
{
    app_desc = app_main(argc, argv);

    int result = SDL_Init(SDL_INIT_VIDEO);
    if (result != 0)
        log_error("%s", SDL_GetError());

    load_configs();
    create_window();
    image_init_decoders();

    app_desc.init();

    float delta_time = 0.005f;
    uint64_t last_tick = SDL_GetPerformanceCounter();

    while (!c_quit->bool_value)
    {
        handle_window_events();
        handle_input_events();

        app_desc.update(delta_time);

        app_desc.draw();

        uint64_t tick = SDL_GetPerformanceCounter();
        uint64_t delta_tick = tick - last_tick;
        last_tick = tick;
        delta_time = (float)(((double)delta_tick * 1000) / (double)SDL_GetPerformanceFrequency());
    }

    config_save(config_file_name);

    app_desc.quit();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

static void load_configs(void)
{
    config_load("config.txt");

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
    const int32_t position = SDL_WINDOWPOS_UNDEFINED;

    bool borderless = c_window_borderless->bool_value;
    bool fullscreen = c_window_fullscreen->bool_value;

    uint32_t flags = SDL_WINDOW_ALLOW_HIGHDPI;
    if (fullscreen && borderless) flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN_DESKTOP;
    else if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

    window = SDL_CreateWindow(
            app_desc.app_name, position, position,
            c_window_width->int_value, c_window_height->int_value, flags);
    if (!window)
        log_error("%s", SDL_GetError());
}

static void handle_window_events(void)
{
    SDL_PumpEvents();

    SDL_Event window_events[8];
    const int count = SDL_PeepEvents(window_events, 8, SDL_GETEVENT, SDL_QUIT, SDL_SYSWMEVENT);

    for (int i = 0; i < count; i++)
    {
        SDL_Event* event = &window_events[i];
        if (event->type == SDL_QUIT)
        {
            c_quit->bool_value = true;
        }
    }
}