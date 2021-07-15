#pragma once

typedef struct WbRenderer WbRenderer;
typedef struct WbTexture WbTexture;
typedef struct WbConfig WbConfig;
typedef struct WbWindow WbWindow;
typedef struct WbRect WbRect;

extern WbConfig* c_render_vsync;
extern WbConfig* c_render_scale;

WbRenderer* wbCreateRenderer(const WbWindow* window);
void wbDestroyRenderer(WbRenderer* renderer);

WbTexture* wbCreateTexture(const WbRenderer* renderer, const char* file_name);

void wbRendererDraw(const WbRenderer* renderer);
void wbRendererPresent(const WbRenderer* renderer);

void wbRenderSelectionBox(const WbRenderer* renderer, const WbRect* rect);

