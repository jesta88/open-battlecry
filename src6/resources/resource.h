#pragma once

#include "std.h"

typedef enum
{
    WB_RESOURCE_STATE_FREE,
    WB_RESOURCE_STATE_ALLOCATED,
    WB_RESOURCE_STATE_INITIALIZED,
    WB_RESOURCE_STATE_PENDING
} wb_resource_state;

typedef struct
{
    u32 id;
    wb_resource_state resource_state;
} wb_pool_slot;

typedef struct wb_resource_pool wb_resource_pool;