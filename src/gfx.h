#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct gfx_config
{
    const char* window_title;
    uint32_t window_width;
    uint32_t window_height;
    bool enable_validation;
} gfx_config;

bool gfx_init(const gfx_config* config);
void gfx_shutdown(void);

// Returns false when the application should quit.
bool gfx_poll_events(void);

// Returns false if the frame should be skipped (minimized / swapchain recreation).
bool gfx_begin_frame(void);
void gfx_end_frame(void);

void gfx_get_extent(uint32_t* w, uint32_t* h);

// Create a texture from RGBA pixels. Returns index into bindless array, or UINT32_MAX on failure.
uint32_t gfx_create_texture(uint32_t width, uint32_t height, const uint8_t* rgba_pixels);

// Draw a sprite. Appends to the current frame's batch (flushed in gfx_end_frame).
// color: packed RGBA (0xFFFFFFFF = white/no tint)
void gfx_draw_sprite(float x, float y, float w, float h, uint32_t texture_index, uint32_t color);

// Draw a sub-region of a sprite texture.
// uv_x, uv_y: top-left corner in [0,1]; uv_w, uv_h: extent in [0,1]
void gfx_draw_sprite_region(float x, float y, float w, float h,
                            uint32_t texture_index, uint32_t color,
                            float uv_x, float uv_y, float uv_w, float uv_h);

// Set camera offset (pixel coordinates)
void gfx_set_camera(float x, float y);
void gfx_get_camera(float* x, float* y);

// Input state (updated each frame during gfx_poll_events)
typedef struct
{
    bool mouse_left_pressed;    // true on the frame the button went down
    bool mouse_right_pressed;
    bool mouse_left_released;   // true on the frame the button was released
    bool mouse_right_released;
    bool mouse_left_held;
    bool mouse_right_held;
    float mouse_x, mouse_y;    // screen coordinates

    // Keyboard (true on the frame the key went down, ignores repeats)
    bool key_escape_pressed;
    bool key_enter_pressed;
    bool key_up_pressed;
    bool key_down_pressed;
    bool key_b_pressed;
    bool key_1_pressed;
    bool key_2_pressed;
    bool key_3_pressed;
    bool key_4_pressed;

    // Text input (filled by SDL_EVENT_TEXT_INPUT when text input is active)
    char text_input[32];
    uint8_t text_input_len; // bytes written this frame (0 if none)

    // Editing/console keys (allow key repeat)
    bool key_backspace_pressed;
    bool key_backtick_pressed;
    bool key_tab_pressed;
    bool key_delete_pressed;
    bool key_left_pressed;
    bool key_right_pressed;
    bool key_home_pressed;
    bool key_end_pressed;
    bool key_page_up_pressed;
    bool key_page_down_pressed;
} gfx_input;

const gfx_input* gfx_get_input(void);

// SDL3 text input mode (enables SDL_EVENT_TEXT_INPUT events)
void gfx_start_text_input(void);
void gfx_stop_text_input(void);
