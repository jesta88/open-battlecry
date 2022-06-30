#pragma once

#include "../common/types.h"

typedef struct wb_xcr wb_xcr;

wb_xcr* wb_xcr_load(const char* path);
void wb_xcr_unload(wb_xcr* xcr);