#pragma once

#include "../types.h"

typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;

typedef struct WbClient
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* screen_render_target;

    bool quit;
} WbClient;

extern WbConfig* c_window_width;
extern WbConfig* c_window_height;
extern WbConfig* c_window_fullscreen;
extern WbConfig* c_window_borderless;

extern WbConfig* c_render_vsync;
extern WbConfig* c_render_scale;

extern WbConfig* c_audio_master_volume;
extern WbConfig* c_audio_music_volume;
extern WbConfig* c_audio_sfx_volume;
extern WbConfig* c_audio_voice_volume;
extern WbConfig* c_audio_ambient_volume;

void wbClientRun(void);
