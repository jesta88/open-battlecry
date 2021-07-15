#include "window.h"
#include "renderer.h"
#include "input.h"
#include "audio.h"
#include "../shared/init.h"
#include "../shared/time.inl"
#include "../shared/config.h"
#include <SDL2/SDL_events.h>

WbConfig* c_quit;

WbConfig* c_window_width;
WbConfig* c_window_height;
WbConfig* c_window_fullscreen;
WbConfig* c_window_borderless;

WbConfig* c_render_vsync;
WbConfig* c_render_scale;

WbConfig* c_audio_master_volume;
WbConfig* c_audio_music_volume;
WbConfig* c_audio_sfx_volume;
WbConfig* c_audio_voice_volume;
WbConfig* c_audio_ambient_volume;

static const char* config_file_name = "config.txt";

static void handleMouseEvents(void);
static void handleKeyboardEvents(void);

int main(int argc, char* argv[])
{
    s_init();

    wbConfigLoad(config_file_name);

    c_quit = wbConfigBool("c_quit", false, 0);

    c_window_width = wbConfigInt("c_window_width", 1280, WB_CONFIG_SAVE);
    c_window_height = wbConfigInt("c_window_height", 720, WB_CONFIG_SAVE);
    c_window_fullscreen = wbConfigBool("c_window_fullscreen", false, WB_CONFIG_SAVE);
    c_window_borderless = wbConfigBool("c_window_borderless", false, WB_CONFIG_SAVE);

    c_render_vsync = wbConfigBool("c_render_vsync", false, WB_CONFIG_SAVE);
    c_render_scale = wbConfigFloat("c_render_scale", 1.0f, WB_CONFIG_SAVE);

    c_audio_master_volume = wbConfigFloat("c_audio_master_volume", 1.0f, WB_CONFIG_SAVE);
    c_audio_music_volume = wbConfigFloat("c_audio_music_volume", 1.0f, WB_CONFIG_SAVE);
    c_audio_sfx_volume = wbConfigFloat("c_audio_sfx_volume", 1.0f, WB_CONFIG_SAVE);
    c_audio_ambient_volume = wbConfigFloat("c_audio_ambient_volume", 1.0f, WB_CONFIG_SAVE);
    c_audio_voice_volume = wbConfigFloat("c_audio_voice_volume", 1.0f, WB_CONFIG_SAVE);

    WbWindow* window = wbCreateWindow("Open Battlecry");
    WbRenderer* renderer = wbCreateRenderer(window);

    char title[128];
    uint64_t last_tick = s_get_tick();

    while (!c_quit->bool_value)
    {
        // Events
        wbHandleWindowEvents();
        wbHandleInputEvents();

        // Update

        // Render
        wbRendererDraw(renderer);
        wbRendererPresent(renderer);

        // Stats
        uint64_t frame_ticks = s_ticks_since(&last_tick);
        sprintf(title, "Open Battlecry - %.2f ms", s_get_ms(frame_ticks));

        wbWindowSetTitle(window, title);
    }

    wbConfigSave(config_file_name);

    wbDestroyRenderer(renderer);
    wbDestroyWindow(window);

    return 0;
}
