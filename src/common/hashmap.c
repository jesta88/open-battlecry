#include "hashmap.h"
#include "random.h"
#include "log.h"
#define XXH_NO_STREAM
#define XXH_INLINE_ALL
#include "../third_party/xxhash.h"
#include <assert.h>
#include <limits.h>
#include <malloc.h>

#define INVALID_HASH 0
#define APPROX_85_PERCENT(x)    (((x) * 870) >> 10)

#ifdef _MSC_VER
#include <intrin.h>
#include <string.h>

s32 __inline clz(u32 value)
{
    unsigned long leading_zero = 0;
    if (_BitScanReverse(&leading_zero, value))
        return 31 - (s32) leading_zero;
    else
        return 32;
}
#endif

static inline s32 fls(u32 x)
{
#ifdef _MSC_VER
    return x ? (s32) (sizeof(int) * CHAR_BIT) - clz(x) : 0;
#else
    return x ? (s32) (sizeof(int) * CHAR_BIT) - __builtin_clz(x) : 0;
#endif
}

static inline u64 fast_div32_init(u32 div)
{
    u64 mt;
    u8 s1, s2;
    s32 l;

    l = fls(div - 1);
    mt = (u64) (0x100000000ULL * ((1ULL << l) - div));
    s1 = (l > 1) ? 1U : (u8) l;
    s2 = (l == 0) ? 0 : (u8) (l - 1);
    return (u64) (mt / div + 1) << 32 | (u64) s1 << 8 | s2;
}

static inline u32 fast_div32(u32 v, u64 div_info)
{
    const u32 m = div_info >> 32;
    const unsigned s1 = (div_info & 0x0000ff00) >> 8;
    const unsigned s2 = (div_info & 0x000000ff);
    const u32 t = (u32) (((u64) v * m) >> 32);
    return (t + ((v - t) >> s1)) >> s2;
}

static inline u32 fast_rem32(u32 v, u32 div, u64 div_info)
{
    return v - div * fast_div32(v, div_info);
}

static bool validate_probe_sequence_length(hashmap_t* hash_map, const hashmap_bucket_t* bucket, u32 i)
{
    u32 base_i = fast_rem32(bucket->key_hash, hash_map->capacity, hash_map->div_info);
    u32 diff = (base_i > i) ? hash_map->capacity - base_i + i : i - base_i;
    return bucket->key_hash == UINT32_MAX || diff == bucket->probe_sequence_length;
}

void wbHashMapInit(hashmap_t* hash_map, u16 capacity)
{
    assert(hash_map != NULL);
    assert(capacity > 1);
    assert(capacity < UINT16_MAX);

    hash_map->capacity = capacity;
    hash_map->count = 0;

    WbRng random = { 0 };
    // TODO: Seed with time
    wb_rng_init(&random, 0);
    hash_map->seed = wb_rng_uint(&random);

    hash_map->buckets = calloc(capacity, sizeof(hashmap_bucket_t));
    assert(hash_map->buckets != NULL);

    hash_map->div_info = fast_div32_init(capacity);
}

void wbHashMapFree(hashmap_t* hash_map)
{
    assert(hash_map != NULL);

    free(hash_map->buckets);
    *hash_map = (hashmap_t){ 0 };
}

void wbHashMapAdd(hashmap_t* hash_map, const char* key, u16 value)
{
    // Should increase the size if we hit this assert
    assert(hash_map->count <= APPROX_85_PERCENT(hash_map->capacity));
    assert(key);
    assert(value < UINT16_MAX);

    u64 key_hash = INVALID_HASH;
    do
    {
        key_hash = XXH3_64bits(key, strlen(key));
    }
    while (key_hash == INVALID_HASH);

    hashmap_bucket_t* bucket, entry;
    u32 i;

    entry.key_hash = key_hash;
    entry.value = value;
    entry.probe_sequence_length = 0;

    /*
     * From the paper: "when inserting, if a record probes a location
     * that is already occupied, the record that has traveled longer
     * in its probe sequence keeps the location, and the other one
     * continues on its probe sequence" (page 12).
     *
     * Basically: if the probe sequence length (PSL) of the element
     * being inserted is greater than PSL of the element in the bucket,
     * then swap them and continue.
     */
    i = fast_rem32(key_hash, hash_map->capacity, hash_map->div_info);
probe:
    bucket = &hash_map->buckets[i];
    if (bucket->key_hash != INVALID_HASH)
    {
        assert(validate_probe_sequence_length(hash_map, bucket, i));

        // Duplicate key
        if (bucket->key_hash == key_hash)
        {
            wb_log_error("Duplicate hash map key: %s", key);
            assert(0);
        }

        /*
         * We found a "rich" bucket.  Capture its location.
         */
        if (entry.probe_sequence_length > bucket->probe_sequence_length)
        {
            hashmap_bucket_t temp_bucket;

            /*
             * Place our key-value pair by swapping the "rich"
             * bucket with our entry.  Copy the structures.
             */
            temp_bucket = entry;
            entry = *bucket;
            *bucket = temp_bucket;
        }
        entry.probe_sequence_length++;

        /* Continue to the next bucket. */
        assert(validate_probe_sequence_length(hash_map, bucket, i));
        i = fast_rem32(i + 1, hash_map->capacity, hash_map->div_info);
        goto probe;
    }

    /*
     * Found a free bucket: insert the entry.
     */
    *bucket = entry; // copy
    hash_map->count++;

    assert(validate_probe_sequence_length(hash_map, bucket, i));
}

u16 wbHashMapGet(hashmap_t* hash_map, const char* key)
{
    assert(key != NULL);

    u64 key_hash = INVALID_HASH;
    do
    {
        key_hash = XXH3_64bits(key, strlen(key));
    }
    while (key_hash == INVALID_HASH);
    u32 n = 0, i = fast_rem32(key_hash, hash_map->capacity, hash_map->div_info);
    hashmap_bucket_t* bucket;

    /*
     * Lookup is a linear probe.
     */
probe:
    bucket = &hash_map->buckets[i];
    assert(validate_probe_sequence_length(hash_map, bucket, i));

    if (bucket->key_hash == key_hash)
    {
        return bucket->value;
    }

    /*
     * Stop probing if we hit an empty bucket; also, if we hit a
     * bucket with PSL lower than the distance from the engine location,
     * then it means that we found the "rich" bucket which should
     * have been captured, if the key was inserted -- see the central
     * point of the algorithm in the insertion function.
     */
    if (bucket->key_hash == INVALID_HASH || n > bucket->probe_sequence_length)
    {
        return UINT16_MAX;
    }
    n++;

    /* Continue to the next bucket. */
    i = fast_rem32(i + 1, hash_map->capacity, hash_map->div_info);
    goto probe;
}

u16 wbHashMapRemove(hashmap_t* hash_map, const char* key)
{
    assert(key != NULL);

    u64 key_hash = INVALID_HASH;
    do
    {
        key_hash = XXH3_64bits(key, strlen(key));
    }
    while (key_hash == INVALID_HASH);

    u32 n = 0, i = fast_rem32(key_hash, hash_map->capacity, hash_map->div_info);
    hashmap_bucket_t* bucket;
    u16 value;
probe:
    /*
     * The same probing logic as in the lookup function.
     */
    bucket = &hash_map->buckets[i];
    if (bucket->key_hash == INVALID_HASH || n > bucket->probe_sequence_length)
    {
        return UINT16_MAX;
    }
    assert(validate_probe_sequence_length(hash_map, bucket, i));

    if (bucket->key_hash != key_hash)
    {
        // Continue to the next bucket
        i = fast_rem32(i + 1, hash_map->capacity, hash_map->div_info);
        n++;
        goto probe;
    }

    value = bucket->value;
    hash_map->count--;

    /*
     * The probe sequence must be preserved in the deletion case.
     * Use the backwards-shifting method to maintain low variance.
     */
    for (;;)
    {
        hashmap_bucket_t* nbucket;

        bucket->key_hash = INVALID_HASH;

        i = fast_rem32(i + 1, hash_map->capacity, hash_map->div_info);
        nbucket = &hash_map->buckets[i];
        assert(validate_probe_sequence_length(hash_map, nbucket, i));

        /*
         * Stop if we reach an empty bucket or hit a key which
         * is in its engine (original) location.
         */
        if (nbucket->key_hash == INVALID_HASH || nbucket->probe_sequence_length == 0)
        {
            break;
        }

        nbucket->probe_sequence_length--;
        *bucket = *nbucket;
        bucket = nbucket;
    }

    return value;
}
