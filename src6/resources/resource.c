#include "resource.h"
#include <assert.h>
#include <stdlib.h>

enum
{
	INVALID_ID = 0,
	INVALID_INDEX = 0,

	INDEX_SHIFT = 16,
	INDEX_MASK = (1 << INDEX_SHIFT) - 1
};

struct wb_resource_pool
{
    int size;
    int queue_top;
    int* free_index_queue;
    u16* generations;
};

static int get_index(u32 id)
{
	return (int) (id & INDEX_MASK);
}

static void set_index(wb_resource_pool* resource_pool, wb_pool_slot* pool_slot, int index)
{
	u32 generation = ++resource_pool->generations[index];
    pool_slot->id = (generation << INDEX_SHIFT) | (index & INDEX_MASK);
    pool_slot->resource_state = WB_RESOURCE_STATE_ALLOCATED;
}

void wb_resource_pool_allocate(wb_resource_pool* resource_pool, int count)
{
	resource_pool->size = count + 1;
	resource_pool->queue_top = 0;
	resource_pool->free_index_queue = calloc((size_t)count, sizeof(int));
	assert(resource_pool->free_index_queue);
	resource_pool->generations = calloc((size_t)resource_pool->size, sizeof(u16));
	assert(resource_pool->generations);
	for (int i = resource_pool->size; i >= 1; i--)
	{
		resource_pool->free_index_queue[resource_pool->queue_top++] = i;
	}
}

void wb_resource_pool_free(wb_resource_pool* resource_pool)
{
	free(resource_pool->free_index_queue);
	free(resource_pool->generations);
	resource_pool->free_index_queue = 0;
	resource_pool->generations = 0;
	resource_pool->size = 0;
	resource_pool->queue_top = 0;
}

int wb_resource_pool_get_index(wb_resource_pool* resource_pool)
{
	if (resource_pool->queue_top > 0)
	{
		int index = resource_pool->free_index_queue[--resource_pool->queue_top];
		return index;
	}
	return INVALID_INDEX;
}

void wb_resource_pool_release_index(wb_resource_pool* resource_pool, int index)
{
	resource_pool->free_index_queue[resource_pool->queue_top++] = index;
}
