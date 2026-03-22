#include "args.h"
#include "system.h"

#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static bool          app_paused = false;
static SDL_Window*   app_window = NULL;
static SDL_Renderer* app_renderer = NULL;

static void at_exit(void) { SDL_Quit(); }

static int sdl_init(void)
{
    const int sdl_version = SDL_GetVersion();

    SDL_Log("SDL version %i.%i.%i\n", SDL_VERSIONNUM_MAJOR(sdl_version), SDL_VERSIONNUM_MINOR(sdl_version),
            SDL_VERSIONNUM_MICRO(sdl_version));

    if (SDL_Init(0) < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return 1;
    }

#ifdef _DEBUG
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_DEBUG);
#endif

    atexit(at_exit);

    return 0;
}

void frame(uint64_t ticks) {}

int main(int argc, char* argv[])
{
    args_init(argc, argv);

    if (sdl_init() != 0)
        exit(1);

    if (sys_init() != 0)
        exit(1);

    SDL_Log("Detected %d CPUs.\n", SDL_GetNumLogicalCPUCores());

    if (!SDL_CreateWindowAndRenderer("Hello World", 800, 600, SDL_WINDOW_FULLSCREEN, &app_window, &app_renderer))
    {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        exit(1);
    }

    uint64_t previous_tick = SDL_GetPerformanceCounter();

    while (1)
    {
        const bool has_focus = (SDL_GetWindowFlags(app_window) & (SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS)) != 0;
        if (!has_focus || app_paused)
            SDL_Delay(16);

        const bool is_minimized = (SDL_GetWindowFlags(app_window) & SDL_WINDOW_MINIMIZED) != 0;
        if (is_minimized)
            SDL_Delay(32);

        const uint64_t new_tick = SDL_GetPerformanceCounter();
        const uint64_t ticks = new_tick - previous_tick;

        frame(ticks);

        previous_tick = new_tick;
    }

    SDL_Quit();
    return 0;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS; /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    const char* message = "Hello World!";
    int         w = 0, h = 0;
    float       x, y;
    const float scale = 4.0f;

    /* Center the message and scale it up */
    SDL_GetRenderOutputSize(app_renderer, &w, &h);
    SDL_SetRenderScale(app_renderer, scale, scale);
    x = (w / scale - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(message)) / 2;
    y = (h / scale - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;

    /* Draw the message */
    SDL_SetRenderDrawColor(app_renderer, 0, 0, 0, 255);
    SDL_RenderClear(app_renderer);
    SDL_SetRenderDrawColor(app_renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(app_renderer, x, y, message);
    SDL_RenderPresent(app_renderer);

    return SDL_APP_CONTINUE;
}
