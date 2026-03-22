#pragma once

#include "memory.h"

typedef void* (*wk_realloc)(void* ptr, usize new_size, usize old_size, usize align);

void wk_memory_init(void);