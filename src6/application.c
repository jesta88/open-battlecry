#include "application.h"

#ifdef _WIN32
typedef void* HWND;
typedef void* HINSTANCE;
#else
typedef struct SDL_Window SDL_Window;
#endif

typedef struct app_window
{
    const char* title;
    uint32_t width;
    uint32_t height;
    uint32_t windowed_width;
    uint32_t windowed_height;
    bool fullscreen;
    bool maximized;
    bool needs_resize;

#ifdef _WIN32
    HWND hwnd;
    HINSTANCE hinstance;
#else
    SDL_Window* sdl_window;
#endif
} app_window;

typedef struct app
{
    const char* name;
    bool should_quit;
    int argc;
    char** argv;

    app_window window;

    bool (*init)(void);
    void (*quit)(void);

    bool (*load_resources)(void);
    void (*unload_resources)(void);

    void (*update)(float delta_time);
    void (*draw)(void);
} app;

static app s_app;
