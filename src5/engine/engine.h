#pragma once

typedef struct wk_engine_state wk_engine_state;

typedef struct
{
    float target_tickrate;
} wk_engine_desc;

typedef struct
{
    wk_engine_state* (*init)(wk_engine_desc*);
    int (*quit)(wk_engine_state*);
    int (*unload)(wk_engine_state*);
    int (*reload)(wk_engine_state*);
    void (*update)(wk_engine_state*, float);
    int (*events)(wk_engine_state*, void*);
    void (*draw)(wk_engine_state*, float);
    void (*lateupdate)(wk_engine_state*, float);
    void (*start_frame)(wk_engine_state*);
    void (*end_frame)(wk_engine_state*);
} wk_engine_api;

int wk_init_engine(const wk_engine_desc* engine_desc);
void wk_quit_engine(void);

#if WK_PLATFORM_WINDOWS
void wk_set_win32_hinstance(void* hinstance);
void* wk_get_win32_hinstance(void);
#endif