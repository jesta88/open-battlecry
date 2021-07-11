#include "engine.h"
#include "input.h"
#include "bits.h"
#include "log.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

enum
{
    WB_MAX_UNIT_SPRITES = 2048,
    WB_MAX_KEYBOARD_EVENTS = 64,
    WB_MAX_MOUSE_EVENTS = 256,
};

static bool quit;

static SDL_Window* window;
static SDL_Renderer* renderer;
static SDL_Texture* screenTexture;

static SDL_Rect unit_frame_rects[WB_MAX_UNIT_SPRITES];
static SDL_Rect unit_rects[WB_MAX_UNIT_SPRITES];
static SDL_Texture* unit_textures[WB_MAX_UNIT_SPRITES];

// Input
static WbBitset256 keys_bitset;
static WbBitset8 mouse_bitset;
static WbBitset8 mouse_pressed_bitset;
static WbBitset8 mouse_released_bitset;
static int32_t mouse_delta_x;
static int32_t mouse_delta_y;
static int32_t mouse_wheel_x;
static int32_t mouse_wheel_y;

// Camera
static int32_t camera_x;
static int32_t camera_y;

// Selection box
static bool update_selection_box;
static bool render_selection_box;
static int32_t selection_box_anchor_x;
static int32_t selection_box_anchor_y;
static SDL_Rect selection_box_rect;

static void handleKeyboardEvents(void);

static void handleMouseEvents(void);

void wbRun(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }
    WB_LOG_INFO("SDL initialized.");

    if (IMG_Init(IMG_INIT_PNG) < 0)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }
    WB_LOG_INFO("SDL_Image initialized.");

    uint32_t flags = SDL_WINDOW_ALLOW_HIGHDPI;
    window = SDL_CreateWindow("Battlecry", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, flags);
    if (window == NULL)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }
    WB_LOG_INFO("Window created.");

    flags = SDL_RENDERER_ACCELERATED;
    renderer = SDL_CreateRenderer(window, -1, flags);
    if (renderer == NULL)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }
    WB_LOG_INFO("Renderer created.");

    screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_TARGET, 1280, 720);
    if (screenTexture == NULL)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }

    SDL_Surface* unit_surface = IMG_Load("../assets/images/sides/dwarves/ADBX.png");
    if (unit_surface == NULL)
    {
        WB_LOG_ERROR("%s", IMG_GetError());
    }

    unit_textures[0] = SDL_CreateTextureFromSurface(renderer, unit_surface);
    if (unit_textures[0] == NULL)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }
    SDL_FreeSurface(unit_surface);

    if (SDL_QueryTexture(unit_textures[0], NULL, NULL, &unit_frame_rects[0].w, &unit_frame_rects[0].h) < 0)
    {
        WB_LOG_ERROR("%s", SDL_GetError());
    }

    unit_frame_rects[0].w /= 2;
    unit_frame_rects[0].h /= 8;

    unit_rects[0].x = 40;
    unit_rects[0].y = 80;
    unit_rects[0].w = unit_frame_rects[0].w;
    unit_rects[0].h = unit_frame_rects[0].h;

    WB_LOG_INFO("Starting main loop.");

    while (!quit)
    {
        uint32_t start_tick = SDL_GetTicks();

        if (SDL_QuitRequested())
        {
            quit = true;
            break;
        }

        handleKeyboardEvents();
        handleMouseEvents();

        // Update
        if (wbCheckBit8(&mouse_pressed_bitset, WB_MOUSE_BUTTON_LEFT))
        {
            render_selection_box = false;
            SDL_GetMouseState(&selection_box_anchor_x, &selection_box_anchor_y);
            update_selection_box = true;
        }
        if (wbCheckBit8(&mouse_released_bitset, WB_MOUSE_BUTTON_LEFT))
        {
            update_selection_box = false;
            render_selection_box = false;
        }
        if (update_selection_box && wbCheckBit8(&mouse_bitset, WB_MOUSE_BUTTON_LEFT))
        {
            int32_t mouse_x, mouse_y;
            SDL_GetMouseState(&mouse_x, &mouse_y);

            if (mouse_x < selection_box_anchor_x)
            {
                selection_box_rect.x = mouse_x;
                selection_box_rect.w = selection_box_anchor_x - mouse_x;
            }
            else
            {
                selection_box_rect.x = selection_box_anchor_x;
                selection_box_rect.w = mouse_x - selection_box_anchor_x;
            }

            if (mouse_y < selection_box_anchor_y)
            {
                selection_box_rect.y = mouse_y;
                selection_box_rect.h = selection_box_anchor_y - mouse_y;
            }
            else
            {
                selection_box_rect.y = selection_box_anchor_y;
                selection_box_rect.h = mouse_y - selection_box_anchor_y;
            }

            if (selection_box_rect.w > 2 || selection_box_rect.h > 2)
                render_selection_box = true;
            else
                render_selection_box = false;
        }


        // Render
        SDL_SetRenderTarget(renderer, screenTexture);
        SDL_SetRenderDrawColor(renderer, 0xA5, 0xB7, 0xA4, 0xFF);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, unit_textures[0], &unit_frame_rects[0], &unit_rects[0]);

        if (render_selection_box)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0xFF, 0, 0xFF);
            SDL_RenderDrawRect(renderer, &selection_box_rect);
            SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0, 0xFF);
            SDL_RenderDrawPoint(renderer, 100, 100);
        }

        SDL_SetRenderTarget(renderer, NULL);

        SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
        SDL_RenderPresent(renderer);

        uint32_t end_tick = SDL_GetTicks();
        char title[128];
        uint32_t ms = end_tick - start_tick;
        sprintf(title, "Battlecry - %i ms", ms);
        SDL_SetWindowTitle(window, title);
    }

    WB_LOG_INFO("Quitting.");

    SDL_DestroyTexture(unit_textures[0]);

    SDL_DestroyTexture(screenTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static void handleKeyboardEvents(void)
{
    SDL_Event events[WB_MAX_KEYBOARD_EVENTS];
    const int count = SDL_PeepEvents(
        events, WB_MAX_KEYBOARD_EVENTS,
        SDL_GETEVENT, SDL_KEYDOWN, SDL_TEXTINPUT);

    for (int i = 0; i < count; i++)
    {
        const SDL_Event event = events[i];
        switch (event.type)
        {
            case SDL_KEYDOWN:
                if (event.key.repeat)
                    continue;
                wbSetBit256(&keys_bitset, event.key.keysym.scancode);
                // TODO: Callback
                break;

            case SDL_KEYUP:
                wbClearBit256(&keys_bitset, event.key.keysym.scancode);
                // TODO: Callback
                break;

            default:
                break;
        }
    }
}

static void handleMouseEvents(void)
{
    WbBitset8 previous_mouse_bitset = mouse_bitset;

    SDL_Event events[WB_MAX_MOUSE_EVENTS];
    const int count = SDL_PeepEvents(
        events, WB_MAX_MOUSE_EVENTS,
        SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEWHEEL);

    mouse_delta_x = mouse_delta_y = mouse_wheel_x = mouse_wheel_y = 0;

    for (int i = 0; i < count; ++i)
    {
        const SDL_Event event = events[i];

        switch (event.type)
        {
            case SDL_MOUSEMOTION:
                //                mouseTimeStamp[0] = m_curTime + event.motion.timestamp;
                //                mouseTimeStamp[1] = m_curTime + event.motion.timestamp;
                mouse_delta_x += event.motion.xrel;
                mouse_delta_y += event.motion.yrel;
                break;

            case SDL_MOUSEBUTTONDOWN:
                wbSetBit8(&mouse_bitset, event.button.button - 1);
                // TODO: Callback
                break;

            case SDL_MOUSEBUTTONUP:
                wbClearBit8(&mouse_bitset, event.button.button - 1);
                // TODO: Callback
                break;

            case SDL_MOUSEWHEEL:
                //                mouseTimeStamp[2] = m_curTime + event.wheel.timestamp;
                //                mouseTimeStamp[3] = m_curTime + event.wheel.timestamp;
                mouse_wheel_y += event.wheel.y;
                mouse_wheel_x += event.wheel.x;
                break;
        }
    }

    mouse_pressed_bitset.bits = 0;
    mouse_released_bitset.bits = 0;

    for (int i = 0; i < WB_MOUSE_BUTTON_COUNT; i++)
    {
        if (wbCheckBit8(&mouse_bitset, i) && !wbCheckBit8(&previous_mouse_bitset, i))
        {
            wbSetBit8(&mouse_pressed_bitset, i);
        }
        if (!wbCheckBit8(&mouse_bitset, i) && wbCheckBit8(&previous_mouse_bitset, i))
        {
            wbSetBit8(&mouse_released_bitset, i);
        }
    }
}