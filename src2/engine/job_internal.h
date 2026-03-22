/* job_internal.h - Internal types and platform primitives
 *
 * Not part of public API - do not include directly.
 */

#ifndef JOB_INTERNAL_H
#define JOB_INTERNAL_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif
#include <intrin.h>
#include <windows.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

/*============================================================================
 * Compiler / Platform Macros
 *============================================================================*/

#define JOB_CACHE_LINE 64
#define JOB_PAYLOAD_SIZE 48 /* Bytes available for inline data */
#define JOB_ALIGNAS(x) __declspec(align(x))
#define JOB_INLINE __forceinline
#define JOB_NOINLINE __declspec(noinline)
#define JOB_THREAD_LOCAL __declspec(thread)

#define JOB_ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

/*============================================================================
 * Atomic Primitives (Win32 Interlocked)
 *============================================================================*/

/* Memory barriers */
JOB_INLINE void job_memory_barrier(void)
{
    _mm_mfence();
}

JOB_INLINE void job_compiler_barrier(void)
{
    _ReadWriteBarrier();
}

JOB_INLINE void job_store_barrier(void)
{
    _mm_sfence();
}

JOB_INLINE void job_load_barrier(void)
{
    _mm_lfence();
}

/* Atomic load/store with explicit ordering */
JOB_INLINE int64_t job_atomic_load_relaxed(const volatile int64_t* ptr)
{
    return *ptr;
}

JOB_INLINE int64_t job_atomic_load_acquire(const volatile int64_t* ptr)
{
    int64_t val = *ptr;
    job_compiler_barrier();
    return val;
}

JOB_INLINE void job_atomic_store_relaxed(volatile int64_t* ptr, int64_t val)
{
    *ptr = val;
}

JOB_INLINE void job_atomic_store_release(volatile int64_t* ptr, int64_t val)
{
    job_compiler_barrier();
    *ptr = val;
}

JOB_INLINE int32_t job_atomic_load32_relaxed(const volatile int32_t* ptr)
{
    return *ptr;
}

JOB_INLINE int32_t job_atomic_load32_acquire(const volatile int32_t* ptr)
{
    int32_t val = *ptr;
    job_compiler_barrier();
    return val;
}

JOB_INLINE void job_atomic_store32_relaxed(volatile int32_t* ptr, int32_t val)
{
    *ptr = val;
}

JOB_INLINE void job_atomic_store32_release(volatile int32_t* ptr, int32_t val)
{
    job_compiler_barrier();
    *ptr = val;
}

/* Compare-and-swap */
JOB_INLINE bool job_atomic_cas64(volatile int64_t* ptr, int64_t expected, int64_t desired)
{
    return InterlockedCompareExchange64(ptr, desired, expected) == expected;
}

JOB_INLINE bool job_atomic_cas32(volatile int32_t* ptr, int32_t expected, int32_t desired)
{
    return InterlockedCompareExchange((volatile LONG*) ptr, desired, expected) == expected;
}

JOB_INLINE int64_t job_atomic_exchange64(volatile int64_t* ptr, int64_t val)
{
    return InterlockedExchange64(ptr, val);
}

JOB_INLINE int32_t job_atomic_exchange32(volatile int32_t* ptr, int32_t val)
{
    return InterlockedExchange((volatile LONG*) ptr, val);
}

/* Fetch-and-add */
JOB_INLINE int64_t job_atomic_fetch_add64(volatile int64_t* ptr, int64_t val)
{
    return InterlockedExchangeAdd64(ptr, val);
}

JOB_INLINE int32_t job_atomic_fetch_add32(volatile int32_t* ptr, int32_t val)
{
    return InterlockedExchangeAdd((volatile LONG*) ptr, val);
}

JOB_INLINE int32_t job_atomic_fetch_sub32(volatile int32_t* ptr, int32_t val)
{
    return InterlockedExchangeAdd((volatile LONG*) ptr, -val);
}

JOB_INLINE int32_t job_atomic_increment32(volatile int32_t* ptr)
{
    return InterlockedIncrement((volatile LONG*) ptr);
}

JOB_INLINE int32_t job_atomic_decrement32(volatile int32_t* ptr)
{
    return InterlockedDecrement((volatile LONG*) ptr);
}

/*============================================================================
 * Utility
 *============================================================================*/

JOB_INLINE bool job_is_power_of_two(uint32_t x)
{
    return x && !(x & x - 1);
}

JOB_INLINE uint32_t job_next_power_of_two(uint32_t x)
{
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

/* Fast random for stealing victim selection (xorshift) */
JOB_INLINE uint32_t job_random_next(uint32_t* state)
{
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

/*============================================================================
 * Internal Job Structure (cache-line aligned)
 *============================================================================*/

typedef struct JobCounter JobCounter;
typedef void (*JobFunc)(void* ctx);

typedef struct JOB_ALIGNAS(JOB_CACHE_LINE) Job
{
    JobFunc func;                      /* 8 bytes */
    JobCounter* completion;            /* 8 bytes */
    uint8_t payload[JOB_PAYLOAD_SIZE]; /* 48 bytes */
} Job;                                 /* Total: 64 bytes */

static_assert(sizeof(Job) == JOB_CACHE_LINE, "Job must be cache-line sized");
static_assert(JOB_PAYLOAD_SIZE == 48, "Payload size mismatch");

#endif /* JOB_INTERNAL_H */