#pragma once

#include "../base/math.h"
#include <stdbool.h>

struct window;

extern struct config* c_render_vsync;
extern struct config* c_render_scale;

struct renderer* wbCreateRenderer(const struct window* window);
void wbDestroyRenderer(struct renderer* renderer);

struct texture* wbCreateTexture(const struct renderer* renderer, const char* file_name, bool transparent);

void wbRendererDraw(const struct renderer* renderer);
void wbRendererPresent(const struct renderer* renderer);

void wbRenderSelectionBox(const struct renderer* renderer, const struct rect* rect);

