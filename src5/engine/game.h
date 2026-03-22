#pragma once

typedef struct {
    void (*init)(void);
    void (*update)(float delta_time);
    void (*quit)(void);

    const char* title;
    const char* org;
    float target_tickrate;
} wk_game_desc;

// Implement this in the game library
extern wk_game_desc wk_game_main(int argc, char* argv[]);