#pragma once

#include "types.h"

typedef struct WbRenderer WbRenderer;

void wbInitRenderer(WbRenderer** renderer);
void wbShutdownRenderer(WbRenderer* renderer);

void wbRendererPresent(WbRenderer* renderer);

void wbRenderSelectionBox(WbRenderer* renderer, const WbRect* rect);

