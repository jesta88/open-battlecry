#pragma once

#include <stdint.h>

typedef struct hashmap_bucket_t
{
    uint32_t key_hash;
    uint16_t value;
    uint16_t probe_sequence_length;
} hashmap_bucket_t;

typedef struct hashmap_t
{
    uint16_t count;
    uint16_t capacity;
    uint32_t seed;
    uint64_t div_info;
    hashmap_bucket_t* buckets;
} hashmap_t;

void wbHashMapInit(hashmap_t* hash_map, uint16_t capacity);
void wbHashMapFree(hashmap_t* hash_map);

void wbHashMapAdd(hashmap_t* hash_map, const char* key, uint16_t value);
uint16_t wbHashMapGet(hashmap_t* hash_map, const char* key);
uint16_t wbHashMapRemove(hashmap_t* hash_map, const char* key);
