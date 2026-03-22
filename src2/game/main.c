#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
  #include <windows.h>
  #include <locale.h>
  static void recon_enable_utf8_console(void) {
      // Make C locale and Win32 console use UTF-8. Keep wide Win32 APIs for paths.
      setlocale(LC_ALL, ".UTF-8");
      SetConsoleOutputCP(CP_UTF8);
      SetConsoleCP(CP_UTF8);
  }
#else
  static void recon_enable_utf8_console(void) { /* no-op */ }
#endif

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>      // for SDL_Vulkan_GetDrawableSize (SDL3)
#include <render_api/render_api.h>
#include <renderer/renderer.h>   // recon_renderer_vk_api()

static void sdl_fail(const char* where) {
    fprintf(stderr, "[SDL] %s failed: %s\n", where, SDL_GetError());
    exit(1);
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    recon_enable_utf8_console();

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) { sdl_fail("SDL_Init"); }

    // Window flags: Vulkan + resizable + high-DPI friendly.
    SDL_Window* win = SDL_CreateWindow("Recon Client",
        1280, 720,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!win) sdl_fail("SDL_CreateWindow");

    int drawable_w = 0, drawable_h = 0;
    if (!SDL_GetWindowSizeInPixels(win, &drawable_w, &drawable_h)) {
        // Fallback if extension not present: use window size.
        SDL_GetWindowSize(win, &drawable_w, &drawable_h);
    }

    // Grab the renderer API (statically linked function table).
    const RgApi* rg = recon_renderer_vk_api();
    if (!rg || rg->api_version != RG_API_VERSION) {
        fprintf(stderr, "[Renderer] API version mismatch (renderer=%u, header=%u)\n",
                rg ? rg->api_version : 0u, RG_API_VERSION);
        return 2;
    }

    RgHandle* r = NULL;
    RgCreateInfo ci = {
        .sdl_window = win,
        .width = (uint32_t)drawable_w,
        .height = (uint32_t)drawable_h,
        .enable_validation = true,   // flip off in release if you want
    };
    if (rg->create(&ci, &r) != RG_SUCCESS || !r) {
        fprintf(stderr, "[Renderer] create failed\n");
        return 3;
    }

    bool running = true;
    bool size_dirty = false;

    uint64_t last_us = SDL_GetTicksNS() / 1000ULL;

    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                    // SDL3 sends size in event data; safest is to re-query drawable size.
                    SDL_GetWindowSizeInPixels(win, &drawable_w, &drawable_h);
                    size_dirty = true;
                    break;
                default: break;
            }
        }

        if (size_dirty) {
            rg->resize(r, (uint32_t)drawable_w, (uint32_t)drawable_h);
            size_dirty = false;
        }

        // Timing (microseconds)
        uint64_t now_us = SDL_GetTicksNS() / 1000ULL;
        double dt = (double)(now_us - last_us) / 1e6;
        last_us = now_us;
        (void)dt; // Hook your engine update here.

        // Minimal frame data (clear to dark gray).
        RgFrameData fd = { .clear_rgba = { 0.12f, 0.12f, 0.14f, 1.0f } };

        if (rg->begin_frame(r) != RG_SUCCESS) {
            // On swapchain out-of-date, you could trigger a resize path here.
        }
        rg->draw(r, &fd);
        rg->end_frame(r);
    }

    rg->destroy(r);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}