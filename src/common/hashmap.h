#pragma once

#include "types.h"

typedef struct hashmap_bucket_t
{
    u64 key_hash;
    u32 value;
    u32 probe_sequence_length;
} hashmap_bucket_t;

typedef struct hashmap_t
{
    u32 count;
    u32 capacity;
    u64 seed;
    u64 div_info;
    hashmap_bucket_t* buckets;
} hashmap_t;

void wbHashMapInit(hashmap_t* hash_map, u16 capacity);
void wbHashMapFree(hashmap_t* hash_map);

void wbHashMapAdd(hashmap_t* hash_map, const char* key, u16 value);
u16 wbHashMapGet(hashmap_t* hash_map, const char* key);
u16 wbHashMapRemove(hashmap_t* hash_map, const char* key);
