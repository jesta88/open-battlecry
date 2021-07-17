#include "window.h"
#include "renderer.h"
#include "input.h"
#include "../base/base.h"
#include "../base/time.inl"
#include "../base/config.h"
#include <SDL2/SDL_events.h>

struct config* c_quit;

struct config* c_window_width;
struct config* c_window_height;
struct config* c_window_fullscreen;
struct config* c_window_borderless;

struct config* c_render_vsync;
struct config* c_render_scale;

struct config* c_audio_master_volume;
struct config* c_audio_music_volume;
struct config* c_audio_sfx_volume;
struct config* c_audio_voice_volume;
struct config* c_audio_ambient_volume;

static const char* config_file_name = "config.txt";

static void handle_mouse_events(void);
static void handle_keyboard_events(void);

int main(int argc, char* argv[])
{
    base_init();

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

    struct window* window = c_create_window("Open Battlecry");
    struct renderer* renderer = wbCreateRenderer(window);

    char title[128];
    uint64_t last_tick = time_now();

    struct texture* texture = wbCreateTexture(renderer, "../assets/images/sides/dwarves/ADBX.png", true);

    while (!c_quit->bool_value)
    {
        // Events
        wbHandleWindowEvents();
        c_handle_input_events();

        // Update

        // Render
        wbRendererDraw(renderer);
        wbRendererPresent(renderer);

        // Stats
        uint64_t frame_ticks = time_since(&last_tick);
        sprintf(title, "Open Battlecry - %.2f ms", time_ms(frame_ticks));

        wbWindowSetTitle(window, title);
    }

    config_save(config_file_name);

    wbDestroyRenderer(renderer);
    wbDestroyWindow(window);

    return 0;
}
