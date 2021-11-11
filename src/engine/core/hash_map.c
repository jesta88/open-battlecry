#include "hash_map.h"
#include "random.h"
#include "hash.h"
#include "../../../include/engine/log.h"
#include <limits.h>
#include <malloc.h>
#include <assert.h>
#include <stdbool.h>

#define INVALID_HASH 0
#define APPROX_85_PERCENT(x)    (((x) * 870) >> 10)

#ifdef _MSC_VER
#include <intrin.h>
#include <string.h>

int32_t __inline clz(uint32_t value)
{
    unsigned long leading_zero = 0;
    if (_BitScanReverse(&leading_zero, value))
        return 31 - (int32_t) leading_zero;
    else
        return 32;
}
#endif

static inline int32_t fls(uint32_t x)
{
#ifdef _MSC_VER
    return x ? (int32_t) (sizeof(int) * CHAR_BIT) - clz(x) : 0;
#else
    return x ? (int32_t)(sizeof(int) * CHAR_BIT) - __builtin_clz(x) : 0;
#endif
}

static inline uint64_t fast_div32_init(uint32_t div)
{
    uint64_t mt;
    uint8_t s1, s2;
    int32_t l;

    l = fls(div - 1);
    mt = (uint64_t) (0x100000000ULL * ((1ULL << l) - div));
    s1 = (l > 1) ? 1U : (uint8_t) l;
    s2 = (l == 0) ? 0 : (uint8_t) (l - 1);
    return (uint64_t) (mt / div + 1) << 32 | (uint64_t) s1 << 8 | s2;
}

static inline uint32_t fast_div32(uint32_t v, uint64_t div_info)
{
    const uint32_t m = div_info >> 32;
    const unsigned s1 = (div_info & 0x0000ff00) >> 8;
    const unsigned s2 = (div_info & 0x000000ff);
    const uint32_t t = (uint32_t) (((uint64_t) v * m) >> 32);
    return (t + ((v - t) >> s1)) >> s2;
}

static inline uint32_t fast_rem32(uint32_t v, uint32_t div, uint64_t div_info)
{
    return v - div * fast_div32(v, div_info);
}

static bool validate_probe_sequence_length(hashmap_t* hash_map, const hashmap_bucket_t* bucket, uint32_t i)
{
    uint32_t base_i = fast_rem32(bucket->key_hash, hash_map->capacity, hash_map->div_info);
    uint32_t diff = (base_i > i) ? hash_map->capacity - base_i + i : i - base_i;
    return bucket->key_hash == UINT32_MAX || diff == bucket->probe_sequence_length;
}

void wbHashMapInit(hashmap_t* hash_map, uint16_t capacity)
{
    assert(hash_map != NULL);
    assert(capacity > 1);
    assert(capacity < UINT16_MAX);

    hash_map->capacity = capacity;
    hash_map->count = 0;

    random_t random = {0};
    // TODO: Seed with time
    random_init(&random, 0);
    hash_map->seed = random_uint(&random);

    hash_map->buckets = calloc(capacity, sizeof(hashmap_bucket_t));
    assert(hash_map->buckets != NULL);

    hash_map->div_info = fast_div32_init(capacity);
}

void wbHashMapFree(hashmap_t* hash_map)
{
    assert(hash_map != NULL);

    free(hash_map->buckets);
    *hash_map = (hashmap_t) {0};
}

void wbHashMapAdd(hashmap_t* hash_map, const char* key, uint16_t value)
{
    // Should increase the size if we hit this assert
    assert(hash_map->count <= APPROX_85_PERCENT(hash_map->capacity));
    assert(key);
    assert(value < UINT16_MAX);

    uint32_t key_hash = INVALID_HASH;
    do
    {
        key_hash = hash_string(key);
    } while (key_hash == INVALID_HASH);

    hashmap_bucket_t* bucket, entry;
    uint32_t i;

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
            log_error("Duplicate hash map key: %s", key);
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

uint16_t wbHashMapGet(hashmap_t* hash_map, const char* key)
{
    assert(key != NULL);

    uint32_t key_hash = INVALID_HASH;
    do
    {
        key_hash = hash_string(key);
    } while (key_hash == INVALID_HASH);
    uint32_t n = 0, i = fast_rem32(key_hash, hash_map->capacity, hash_map->div_info);
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

uint16_t wbHashMapRemove(hashmap_t* hash_map, const char* key)
{
    assert(key != NULL);

    uint32_t key_hash = INVALID_HASH;
    do
    {
        key_hash = hash_string(key);
    } while (key_hash == INVALID_HASH);

    uint32_t n = 0, i = fast_rem32(key_hash, hash_map->capacity, hash_map->div_info);
    hashmap_bucket_t* bucket;
    uint16_t value;
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
