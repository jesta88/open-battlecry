// #pragma once
// 
// #include "types.h"
// 
// /******************************************************************************
//  * Engine configs
//  */
// extern struct config* c_quit;
// 
// extern struct config* c_window_width;
// extern struct config* c_window_height;
// extern struct config* c_window_fullscreen;
// extern struct config* c_window_borderless;
// 
// extern struct config* c_render_vsync;
// extern struct config* c_render_scale;
// 
// extern struct config* c_camera_zoom;
// extern struct config* c_camera_speed;
// 
// extern struct config* c_audio_master_volume;
// extern struct config* c_audio_music_volume;
// extern struct config* c_audio_sfx_volume;
// extern struct config* c_audio_voice_volume;
// extern struct config* c_audio_ambient_volume;
// 
// enum
// {
//     MAX_CONFIG_NAME_LENGTH = 23  // Ensures that config is 32 bytes.
// };
// 
// enum config_flags
// {
//     // Values before this are used for private flags
//     CONFIG_SAVE = 1 << 4,
//     CONFIG_DEPRECATED = 1 << 5,
// };
// 
// struct config
// {
//     u32 name_hash;
//     char name[MAX_CONFIG_NAME_LENGTH];
//     u8 flags;
//     union
//     {
//         s32 int_value;
//         float float_value;
//         bool bool_value;
//     };
// };
// 
// _Static_assert(sizeof(struct config) == 32, "config must be 32 bytes.");
// 
// struct config* config_get_int(const char* name, s32 value, u8 flags);
// struct config* config_get_float(const char* name, float value, u8 flags);
// struct config* config_get_bool(const char* name, bool value, u8 flags);
// 
// void config_load(const char* file_name);
// void config_save(const char* file_name);