/* job_internal.h - Internal types and platform primitives
 *
 * Not part of public API - do not include directly.
 *
 * Platform support:
 * - Windows: Win32 Interlocked functions, native threads
 * - Linux: <stdatomic.h>, <threads.h>, POSIX semaphores
 */

#ifndef JOB_INTERNAL_H
#define JOB_INTERNAL_H

#include "common.h"
#include <stdint.h>

/*============================================================================
 * Platform Detection
 *============================================================================*/

#ifdef WBC_PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
    #include <intrin.h>
#else
    #include <stdatomic.h>
    #include <threads.h>
    #include <semaphore.h>
    #include <unistd.h>
    #include <stdlib.h>
    #include <sched.h>
    #include <pthread.h>
    #include <time.h>
    #include <errno.h>

    #if defined(__x86_64__) || defined(__i386__)
        #include <emmintrin.h>  /* For _mm_pause, _mm_mfence etc. */
    #elif defined(__aarch64__)
        /* ARM64 uses different intrinsics */
    #endif
#endif

/*============================================================================
 * Compiler / Platform Macros
 *============================================================================*/

/* Aligned allocation */
#ifdef WBC_PLATFORM_WINDOWS
    #define job_aligned_alloc(align, size)  _aligned_malloc((size), (align))
    #define job_aligned_free(ptr)           _aligned_free(ptr)
#else
    #define job_aligned_alloc(align, size)  aligned_alloc((align), (size))
    #define job_aligned_free(ptr)           free(ptr)
#endif

/*============================================================================
 * CPU Pause / Yield
 *============================================================================*/

WBC_INLINE void job_cpu_pause(void) {
#ifdef WBC_PLATFORM_WINDOWS
    YieldProcessor();
#elif defined(__x86_64__) || defined(__i386__)
    _mm_pause();
#elif defined(__aarch64__)
    __asm__ __volatile__("yield" ::: "memory");
#else
    /* Fallback: just a compiler barrier */
    __asm__ __volatile__("" ::: "memory");
#endif
}

/*============================================================================
 * Memory Barriers
 *============================================================================*/

#ifdef WBC_PLATFORM_WINDOWS

WBC_INLINE void job_memory_barrier(void) {
    _mm_mfence();
}

WBC_INLINE void job_compiler_barrier(void) {
    _ReadWriteBarrier();
}

WBC_INLINE void job_store_barrier(void) {
    _mm_sfence();
}

WBC_INLINE void job_load_barrier(void) {
    _mm_lfence();
}

#else /* POSIX / C11+ */

WBC_INLINE void job_memory_barrier(void) {
    atomic_thread_fence(memory_order_seq_cst);
}

WBC_INLINE void job_compiler_barrier(void) {
    atomic_signal_fence(memory_order_seq_cst);
}

WBC_INLINE void job_store_barrier(void) {
    atomic_thread_fence(memory_order_release);
}

WBC_INLINE void job_load_barrier(void) {
    atomic_thread_fence(memory_order_acquire);
}

#endif

/*============================================================================
 * Atomic Operations - 64-bit
 *============================================================================*/

#ifdef WBC_PLATFORM_WINDOWS

WBC_INLINE int64_t job_atomic_load_relaxed(volatile int64_t *ptr) {
    return *ptr;
}

WBC_INLINE int64_t job_atomic_load_acquire(volatile int64_t *ptr) {
    int64_t val = *ptr;
    job_compiler_barrier();
    return val;
}

WBC_INLINE void job_atomic_store_relaxed(volatile int64_t *ptr, int64_t val) {
    *ptr = val;
}

WBC_INLINE void job_atomic_store_release(volatile int64_t *ptr, int64_t val) {
    job_compiler_barrier();
    *ptr = val;
}

WBC_INLINE bool job_atomic_cas64(volatile int64_t *ptr, int64_t expected, int64_t desired) {
    return InterlockedCompareExchange64(ptr, desired, expected) == expected;
}

WBC_INLINE int64_t job_atomic_exchange64(volatile int64_t *ptr, int64_t val) {
    return InterlockedExchange64(ptr, val);
}

WBC_INLINE int64_t job_atomic_fetch_add64(volatile int64_t *ptr, int64_t val) {
    return InterlockedExchangeAdd64(ptr, val);
}

#else /* POSIX / C11+ */

WBC_INLINE int64_t job_atomic_load_relaxed(volatile int64_t *ptr) {
    return atomic_load_explicit((_Atomic int64_t *)ptr, memory_order_relaxed);
}

WBC_INLINE int64_t job_atomic_load_acquire(volatile int64_t *ptr) {
    return atomic_load_explicit((_Atomic int64_t *)ptr, memory_order_acquire);
}

WBC_INLINE void job_atomic_store_relaxed(volatile int64_t *ptr, int64_t val) {
    atomic_store_explicit((_Atomic int64_t *)ptr, val, memory_order_relaxed);
}

WBC_INLINE void job_atomic_store_release(volatile int64_t *ptr, int64_t val) {
    atomic_store_explicit((_Atomic int64_t *)ptr, val, memory_order_release);
}

WBC_INLINE bool job_atomic_cas64(volatile int64_t *ptr, int64_t expected, int64_t desired) {
    return atomic_compare_exchange_strong_explicit(
        (_Atomic int64_t *)ptr, &expected, desired,
        memory_order_seq_cst, memory_order_seq_cst
    );
}

WBC_INLINE int64_t job_atomic_exchange64(volatile int64_t *ptr, int64_t val) {
    return atomic_exchange_explicit((_Atomic int64_t *)ptr, val, memory_order_seq_cst);
}

WBC_INLINE int64_t job_atomic_fetch_add64(volatile int64_t *ptr, int64_t val) {
    return atomic_fetch_add_explicit((_Atomic int64_t *)ptr, val, memory_order_seq_cst);
}

#endif

/*============================================================================
 * Atomic Operations - 32-bit
 *============================================================================*/

#ifdef WBC_PLATFORM_WINDOWS

WBC_INLINE int32_t job_atomic_load32_relaxed(volatile int32_t *ptr) {
    return *ptr;
}

WBC_INLINE int32_t job_atomic_load32_acquire(volatile int32_t *ptr) {
    int32_t val = *ptr;
    job_compiler_barrier();
    return val;
}

WBC_INLINE void job_atomic_store32_relaxed(volatile int32_t *ptr, int32_t val) {
    *ptr = val;
}

WBC_INLINE void job_atomic_store32_release(volatile int32_t *ptr, int32_t val) {
    job_compiler_barrier();
    *ptr = val;
}

WBC_INLINE bool job_atomic_cas32(volatile int32_t *ptr, int32_t expected, int32_t desired) {
    return InterlockedCompareExchange((volatile LONG *)ptr, desired, expected) == expected;
}

WBC_INLINE int32_t job_atomic_exchange32(volatile int32_t *ptr, int32_t val) {
    return InterlockedExchange((volatile LONG *)ptr, val);
}

WBC_INLINE int32_t job_atomic_fetch_add32(volatile int32_t *ptr, int32_t val) {
    return InterlockedExchangeAdd((volatile LONG *)ptr, val);
}

WBC_INLINE int32_t job_atomic_fetch_sub32(volatile int32_t *ptr, int32_t val) {
    return InterlockedExchangeAdd((volatile LONG *)ptr, -val);
}

WBC_INLINE int32_t job_atomic_increment32(volatile int32_t *ptr) {
    return InterlockedIncrement((volatile LONG *)ptr);
}

WBC_INLINE int32_t job_atomic_decrement32(volatile int32_t *ptr) {
    return InterlockedDecrement((volatile LONG *)ptr);
}

#else /* POSIX / C11+ */

WBC_INLINE int32_t job_atomic_load32_relaxed(volatile int32_t *ptr) {
    return atomic_load_explicit((_Atomic int32_t *)ptr, memory_order_relaxed);
}

WBC_INLINE int32_t job_atomic_load32_acquire(volatile int32_t *ptr) {
    return atomic_load_explicit((_Atomic int32_t *)ptr, memory_order_acquire);
}

WBC_INLINE void job_atomic_store32_relaxed(volatile int32_t *ptr, int32_t val) {
    atomic_store_explicit((_Atomic int32_t *)ptr, val, memory_order_relaxed);
}

WBC_INLINE void job_atomic_store32_release(volatile int32_t *ptr, int32_t val) {
    atomic_store_explicit((_Atomic int32_t *)ptr, val, memory_order_release);
}

WBC_INLINE bool job_atomic_cas32(volatile int32_t *ptr, int32_t expected, int32_t desired) {
    return atomic_compare_exchange_strong_explicit(
        (_Atomic int32_t *)ptr, &expected, desired,
        memory_order_seq_cst, memory_order_seq_cst
    );
}

WBC_INLINE int32_t job_atomic_exchange32(volatile int32_t *ptr, int32_t val) {
    return atomic_exchange_explicit((_Atomic int32_t *)ptr, val, memory_order_seq_cst);
}

WBC_INLINE int32_t job_atomic_fetch_add32(volatile int32_t *ptr, int32_t val) {
    return atomic_fetch_add_explicit((_Atomic int32_t *)ptr, val, memory_order_seq_cst);
}

WBC_INLINE int32_t job_atomic_fetch_sub32(volatile int32_t *ptr, int32_t val) {
    return atomic_fetch_sub_explicit((_Atomic int32_t *)ptr, val, memory_order_seq_cst);
}

WBC_INLINE int32_t job_atomic_increment32(volatile int32_t *ptr) {
    return atomic_fetch_add_explicit((_Atomic int32_t *)ptr, 1, memory_order_seq_cst) + 1;
}

WBC_INLINE int32_t job_atomic_decrement32(volatile int32_t *ptr) {
    return atomic_fetch_sub_explicit((_Atomic int32_t *)ptr, 1, memory_order_seq_cst) - 1;
}

#endif

/*============================================================================
 * 128-bit CAS (for ABA-safe freelist)
 *============================================================================*/

#ifdef WBC_PLATFORM_WINDOWS

/* Windows provides _InterlockedCompareExchange128 */
typedef struct WBC_ALIGNAS(16) JobCAS128 {
    int64_t lo;
    int64_t hi;
} JobCAS128;

WBC_INLINE bool job_atomic_cas128(volatile JobCAS128 *ptr, JobCAS128 *expected, JobCAS128 desired) {
    return _InterlockedCompareExchange128(
        (volatile __int64 *)ptr,
        desired.hi,
        desired.lo,
        (__int64 *)expected
    ) != 0;
}

#else /* POSIX - assume __int128 and cmpxchg16b support */

typedef struct WBC_ALIGNAS(16) JobCAS128 {
    int64_t lo;
    int64_t hi;
} JobCAS128;

WBC_INLINE bool job_atomic_cas128(volatile JobCAS128 *ptr, JobCAS128 *expected, JobCAS128 desired) {
    typedef unsigned __int128 uint128_t;
    uint128_t exp_val = ((uint128_t)(uint64_t)expected->hi << 64) | (uint64_t)expected->lo;
    uint128_t des_val = ((uint128_t)(uint64_t)desired.hi << 64) | (uint64_t)desired.lo;

    bool success = __sync_bool_compare_and_swap(
        (volatile uint128_t *)ptr,
        exp_val,
        des_val
    );

    if (!success) {
        uint128_t current = *(volatile uint128_t *)ptr;
        expected->lo = (int64_t)current;
        expected->hi = (int64_t)(current >> 64);
    }

    return success;
}

#endif /* WBC_PLATFORM_WINDOWS */

/*============================================================================
 * Threading Primitives
 *============================================================================*/

#ifdef WBC_PLATFORM_WINDOWS

typedef HANDLE              JobThread;
typedef HANDLE              JobSemaphore;
typedef CRITICAL_SECTION    JobMutex;
typedef CONDITION_VARIABLE  JobCondVar;

#else /* POSIX / C23 */

typedef thrd_t              JobThread;
typedef mtx_t               JobMutex;
typedef cnd_t               JobCondVar;
typedef sem_t               JobSemaphore;

#endif

/* Thread function signature */
#ifdef WBC_PLATFORM_WINDOWS
typedef DWORD (WINAPI *JobThreadFunc)(void *);
#define JOB_THREAD_RETURN DWORD WINAPI
#define JOB_THREAD_RETURN_VALUE 0
#else
typedef int (*JobThreadFunc)(void *);
#define JOB_THREAD_RETURN int
#define JOB_THREAD_RETURN_VALUE 0
#endif

/*----------------------------------------------------------------------------
 * Thread operations
 *----------------------------------------------------------------------------*/

WBC_INLINE bool job_thread_create(JobThread *thread, JobThreadFunc func, void *arg) {
#ifdef WBC_PLATFORM_WINDOWS
    *thread = CreateThread(NULL, 0, func, arg, 0, NULL);
    return *thread != NULL;
#else
    return thrd_create(thread, (thrd_start_t)func, arg) == thrd_success;
#endif
}

WBC_INLINE void job_thread_join(JobThread thread) {
#ifdef WBC_PLATFORM_WINDOWS
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
#else
    thrd_join(thread, NULL);
#endif
}

WBC_INLINE void job_thread_set_affinity(JobThread thread, uint32_t core) {
#ifdef WBC_PLATFORM_WINDOWS
    if (core < 64) {
        SetThreadAffinityMask(thread, 1ULL << core);
    }
#else
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    pthread_setaffinity_np((pthread_t)thread, sizeof(cpu_set_t), &cpuset);
#endif
}

WBC_INLINE void job_thread_set_priority_high(JobThread thread) {
#ifdef WBC_PLATFORM_WINDOWS
    SetThreadPriority(thread, THREAD_PRIORITY_ABOVE_NORMAL);
#else
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_OTHER);
    pthread_setschedparam((pthread_t)thread, SCHED_OTHER, &param);
#endif
}

WBC_INLINE uint32_t job_get_cpu_count(void) {
#ifdef WBC_PLATFORM_WINDOWS
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif defined(_SC_NPROCESSORS_ONLN)
    long count = sysconf(_SC_NPROCESSORS_ONLN);
    return count > 0 ? (uint32_t)count : 1;
#else
    return 1;
#endif
}

/*----------------------------------------------------------------------------
 * Mutex operations
 *----------------------------------------------------------------------------*/

WBC_INLINE bool job_mutex_init(JobMutex *mutex) {
#ifdef WBC_PLATFORM_WINDOWS
    InitializeCriticalSection(mutex);
    return true;
#else
    return mtx_init(mutex, mtx_plain) == thrd_success;
#endif
}

WBC_INLINE void job_mutex_destroy(JobMutex *mutex) {
#ifdef WBC_PLATFORM_WINDOWS
    DeleteCriticalSection(mutex);
#else
    mtx_destroy(mutex);
#endif
}

WBC_INLINE void job_mutex_lock(JobMutex *mutex) {
#ifdef WBC_PLATFORM_WINDOWS
    EnterCriticalSection(mutex);
#else
    mtx_lock(mutex);
#endif
}

WBC_INLINE void job_mutex_unlock(JobMutex *mutex) {
#ifdef WBC_PLATFORM_WINDOWS
    LeaveCriticalSection(mutex);
#else
    mtx_unlock(mutex);
#endif
}

/*----------------------------------------------------------------------------
 * Condition variable operations
 *----------------------------------------------------------------------------*/

WBC_INLINE bool job_condvar_init(JobCondVar *cond) {
#ifdef WBC_PLATFORM_WINDOWS
    InitializeConditionVariable(cond);
    return true;
#else
    return cnd_init(cond) == thrd_success;
#endif
}

WBC_INLINE void job_condvar_destroy(JobCondVar *cond) {
#ifdef WBC_PLATFORM_WINDOWS
    /* Windows condition variables don't need destruction */
    (void)cond;
#else
    cnd_destroy(cond);
#endif
}

WBC_INLINE void job_condvar_wait(JobCondVar *cond, JobMutex *mutex) {
#ifdef WBC_PLATFORM_WINDOWS
    SleepConditionVariableCS(cond, mutex, INFINITE);
#else
    cnd_wait(cond, mutex);
#endif
}

WBC_INLINE bool job_condvar_wait_timeout(JobCondVar *cond, JobMutex *mutex, uint32_t timeout_ms) {
#ifdef WBC_PLATFORM_WINDOWS
    return SleepConditionVariableCS(cond, mutex, timeout_ms) != 0;
#else
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    return cnd_timedwait(cond, mutex, &ts) == thrd_success;
#endif
}

WBC_INLINE void job_condvar_signal(JobCondVar *cond) {
#ifdef WBC_PLATFORM_WINDOWS
    WakeConditionVariable(cond);
#else
    cnd_signal(cond);
#endif
}

WBC_INLINE void job_condvar_broadcast(JobCondVar *cond) {
#ifdef WBC_PLATFORM_WINDOWS
    WakeAllConditionVariable(cond);
#else
    cnd_broadcast(cond);
#endif
}

/*----------------------------------------------------------------------------
 * Semaphore operations
 *----------------------------------------------------------------------------*/

WBC_INLINE bool job_semaphore_init(JobSemaphore *sem, int32_t initial) {
#ifdef WBC_PLATFORM_WINDOWS
    *sem = CreateSemaphoreW(NULL, initial, INT32_MAX, NULL);
    return *sem != NULL;
#else
    return sem_init(sem, 0, (unsigned int)initial) == 0;
#endif
}

WBC_INLINE void job_semaphore_destroy(JobSemaphore *sem) {
#ifdef WBC_PLATFORM_WINDOWS
    CloseHandle(*sem);
#else
    sem_destroy(sem);
#endif
}

WBC_INLINE void job_semaphore_post(JobSemaphore *sem) {
#ifdef WBC_PLATFORM_WINDOWS
    ReleaseSemaphore(*sem, 1, NULL);
#else
    sem_post(sem);
#endif
}

WBC_INLINE bool job_semaphore_wait_timeout(JobSemaphore *sem, uint32_t timeout_ms) {
#ifdef WBC_PLATFORM_WINDOWS
    return WaitForSingleObject(*sem, timeout_ms) == WAIT_OBJECT_0;
#else
    if (timeout_ms == 0) {
        return sem_trywait(sem) == 0;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }

    int result;
    do {
        result = sem_timedwait(sem, &ts);
    } while (result == -1 && errno == EINTR);

    return result == 0;
#endif
}

/*============================================================================
 * High-Resolution Timer
 *============================================================================*/

WBC_INLINE uint64_t job_get_time_ns(void) {
#ifdef WBC_PLATFORM_WINDOWS
    static LARGE_INTEGER freq = {0};
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (uint64_t)(now.QuadPart * 1000000000ULL / freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
#endif
}

/*============================================================================
 * Utility
 *============================================================================*/

WBC_INLINE bool job_is_power_of_two(uint32_t x) {
    return x && !(x & (x - 1));
}

WBC_INLINE uint32_t job_next_power_of_two(uint32_t x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

/* Fast random for stealing victim selection (xorshift) */
WBC_INLINE uint32_t job_random_next(uint32_t *state) {
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
typedef void (*JobFunc)(void *ctx);

typedef struct WBC_ALIGNAS(WBC_CACHE_LINE) Job {
    JobFunc             func;                       /* 8 bytes */
    JobCounter         *completion;                 /* 8 bytes */
    uint8_t             payload[JOB_PAYLOAD_SIZE];  /* 48 bytes */
} Job;                                              /* Total: 64 bytes */

_Static_assert(sizeof(Job) == WBC_CACHE_LINE, "Job must be cache-line sized");
_Static_assert(JOB_PAYLOAD_SIZE == 48, "Payload size mismatch");

#endif /* JOB_INTERNAL_H */