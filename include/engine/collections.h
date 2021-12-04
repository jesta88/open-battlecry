#pragma once

typedef struct {
    int* free_indices;
    int next;
    int size;
} ws_pool;

void ws_init_pool(ws_pool* pool, int size);
void ws_free_pool(ws_pool* pool);
int ws_pool_get_index(ws_pool* pool);
void ws_pool_return_index(ws_pool* pool, int index);