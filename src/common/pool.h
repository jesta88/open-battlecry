//#pragma once
//
//#include "../common/types.h"
//
//typedef struct {
//    int size;
//    int queue_top;
//    s32* free_indices;
//    u32* generations;
//} wb_handle_pool_32;
//
//void wb_resource_pool_allocate(wb_resource_pool* resource_pool, int count);
//void wb_resource_pool_free(wb_resource_pool* resource_pool);
//int wb_resource_pool_get_index(wb_resource_pool* resource_pool);
//void wb_resource_pool_release_index(wb_resource_pool* resource_pool, int slot_index);