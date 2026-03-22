#include "memory_internal.h"
#include <stdlib.h>
#include <assert.h>

#define WK_PAGE_SIZE 4096
#define WK_MAX_PAGE_COUT 16384
#define WK_SMALL_ALLOC_MAX_SIZE 64
