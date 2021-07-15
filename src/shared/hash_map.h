#pragma once

#include "types.h"

typedef struct WbHashMapBucket
{
    uint32_t key_hash;
    uint16_t value;
    uint16_t probe_sequence_length;
} WbHashMapBucket;

typedef struct WbHashMap
{
    uint16_t count;
    uint16_t capacity;
    uint32_t seed;
    uint64_t div_info;
    WbHashMapBucket* buckets;
} WbHashMap;

void wbHashMapInit(WbHashMap* hash_map, uint16_t capacity);
void wbHashMapFree(WbHashMap* hash_map);

void wbHashMapAdd(WbHashMap* hash_map, const char* key, uint16_t value);
uint16_t wbHashMapGet(WbHashMap* hash_map, const char* key);
uint16_t wbHashMapRemove(WbHashMap* hash_map, const char* key);
