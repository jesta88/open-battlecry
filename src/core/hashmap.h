#pragma once

#include "types.h"

typedef struct hashmap_bucket_t
{
    uint64_t key_hash;
    uint32_t value;
    uint32_t probe_sequence_length;
} hashmap_bucket_t;

typedef struct hashmap_t
{
    uint32_t count;
    uint32_t capacity;
    uint64_t seed;
    uint64_t div_info;
    hashmap_bucket_t* buckets;
} hashmap_t;

void wbHashMapInit(hashmap_t* hash_map, uint16_t capacity);
void wbHashMapFree(hashmap_t* hash_map);

void wbHashMapAdd(hashmap_t* hash_map, const char* key, uint16_t value);
uint16_t wbHashMapGet(hashmap_t* hash_map, const char* key);
uint16_t wbHashMapRemove(hashmap_t* hash_map, const char* key);
