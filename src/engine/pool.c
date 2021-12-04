#include <engine/collections.h>
#include <engine/memory.h>
#include <assert.h>

void ws_init_pool(ws_pool* pool, int size)
{
    assert(pool);
    assert(size >= 1);
    pool->size = size + 1;
    pool->next = 0;
}

void ws_free_pool(ws_pool* pool)
{

}

int ws_pool_get_index(ws_pool* pool)
{
    return 0;
}

void ws_pool_return_index(ws_pool* pool, int index)
{

}
