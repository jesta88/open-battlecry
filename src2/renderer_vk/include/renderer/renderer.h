#pragma once

#include <render_api/render_api.h>

// Returns a pointer to a statically-allocated function table owned by the renderer.
// Safe to cache; thread-safety of calls is up to the renderer implementation.
const RgApi* recon_renderer_vk_api(void);
