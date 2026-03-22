#pragma once
// Minimal, C-only, no SDL headers pulled in. Engine can include this safely.

#include <stdint.h>

// Forward declaration so we can pass an SDL window without including SDL headers.
struct SDL_Window;

// --- Versioning -------------------------------------------------------------
#define RG_API_VERSION_MAJOR 1
#define RG_API_VERSION_MINOR 0
#define RG_API_VERSION_PATCH 0
#define RG_API_VERSION ((RG_API_VERSION_MAJOR<<16)|(RG_API_VERSION_MINOR<<8)|RG_API_VERSION_PATCH)

// --- Result codes -----------------------------------------------------------
typedef enum RgResult {
    RG_SUCCESS = 0,
    RG_ERR_UNKNOWN = -1,
    RG_ERR_BAD_ARG = -2,
    RG_ERR_BACKEND = -3
} RgResult;

// --- Opaque handle to the renderer instance --------------------------------
typedef struct RgHandle RgHandle;

// --- Creation info ----------------------------------------------------------
typedef struct RgCreateInfo {
    // If using SDL for surface creation (recommended), pass the SDL window pointer.
    // The renderer library depends on SDL and will use it internally.
    struct SDL_Window* sdl_window;

    // Initial drawable size in pixels. If zero, the renderer will query from the window.
    uint32_t width;
    uint32_t height;

    // Diagnostics & features
    bool enable_validation;   // Vulkan validation, etc.
} RgCreateInfo;

// --- Per-frame input from the engine (keep tiny on purpose) -----------------
// Expand over time as your engine needs (camera, view/proj, exposure, etc.)
typedef struct RgFrameData {
    float clear_rgba[4];  // simple clear color demo
    // Future: float view[16], proj[16], viewproj[16], etc.
} RgFrameData;

// --- Renderer function table ------------------------------------------------
typedef struct RgApi {
    uint32_t api_version; // == RG_API_VERSION of the renderer

    RgResult (*create)(const RgCreateInfo* ci, RgHandle** out_handle);
    void     (*destroy)(RgHandle* handle);

    // Resize swapchain/drawable in pixels.
    RgResult (*resize)(RgHandle* handle, uint32_t width, uint32_t height);

    // Frame lifecycle; begin/end may block on GPU present depending on backend.
    RgResult (*begin_frame)(RgHandle* handle);
    RgResult (*draw)       (RgHandle* handle, const RgFrameData* frame);
    RgResult (*end_frame)  (RgHandle* handle);
} RgApi;
