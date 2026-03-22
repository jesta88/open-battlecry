/* job_counter.h - Lock-free counter pool for job dependencies
 *
 * Counters serve two purposes:
 * 1. Dependency tracking: A job waits for its dependency counter to reach 0
 * 2. Completion signaling: Jobs decrement a completion counter when done
 *
 * Design:
 * - Fixed pool of counters to avoid runtime allocation
 * - Atomic reference counting for safe recycling
 * - Generation tags to prevent ABA problems
 * - Lock-free allocation and deallocation
 *
 * Lifecycle:
 * 1. Allocate counter with initial value (typically job count)
 * 2. Pass as `completion` to jobs - each job decrements on finish
 * 3. Wait on counter (blocks until value reaches 0)
 * 4. Counter auto-recycles when value=0 AND refcount=0
 */

#ifndef JOB_COUNTER_H
#define JOB_COUNTER_H

#include "job_internal.h"

/*============================================================================
 * Configuration
 *============================================================================*/

#ifndef JOB_COUNTER_POOL_SIZE
#define JOB_COUNTER_POOL_SIZE 4096
#endif

/*============================================================================
 * Types
 *============================================================================*/

/* Counter state packed into 64 bits for atomic operations:
 * - Bits  0-30: Value (31 bits, max ~2 billion)
 * - Bit     31: Reserved
 * - Bits 32-47: Reference count (16 bits)
 * - Bits 48-63: Generation (16 bits, for ABA prevention)
 */
typedef union JobCounterState {
    struct {
        uint32_t    value;          /* Current counter value */
        uint16_t    refcount;       /* Reference count */
        uint16_t    generation;     /* ABA prevention */
    };
    int64_t         packed;         /* For atomic operations */
} JobCounterState;

static_assert(sizeof(JobCounterState) == 8, "JobCounterState must be 64 bits");

/* Counter structure - cache-line aligned to prevent false sharing */
struct JOB_ALIGNAS(JOB_CACHE_LINE) JobCounter {
    volatile int64_t    state;          /* Packed state (atomic) */
    volatile int32_t    waiters;        /* Number of threads waiting */
    int32_t             _pad;
    HANDLE              event;          /* Win32 event for blocking wait */
};

static_assert(sizeof(JobCounter) <= JOB_CACHE_LINE,
               "JobCounter should fit in cache line");

/*============================================================================
 * Pool Management (internal use)
 *============================================================================*/

/* Initialize the global counter pool. Called by job_system_init. */
bool job_counter_pool_init(void);

/* Shutdown the counter pool. Called by job_system_shutdown. */
void job_counter_pool_shutdown(void);

/*============================================================================
 * Public API
 *============================================================================*/

/* Allocate a counter with initial value.
 * Automatically adds 1 to refcount (caller holds a reference).
 * Returns NULL if pool exhausted. */
JobCounter *job_counter_alloc(uint32_t initial_value);

/* Add reference to counter (for passing to another subsystem) */
void job_counter_addref(JobCounter *counter);

/* Release reference. Counter may be recycled if value=0 and refcount=0. */
void job_counter_release(JobCounter *counter);

/* Increment counter value (for dynamic job spawning) */
void job_counter_increment(JobCounter *counter, uint32_t amount);

/* Decrement counter value. Called when a job completes.
 * Wakes waiters if value reaches 0. */
void job_counter_decrement(JobCounter *counter);

/* Check if counter value is zero (non-blocking) */
bool job_counter_is_zero(const JobCounter *counter);

/* Get current counter value (non-blocking, may race) */
uint32_t job_counter_value(const JobCounter *counter);

/* Block until counter reaches zero.
 * Returns immediately if already zero.
 * Safe to call from any thread. */
void job_counter_wait(JobCounter *counter);

/* Block with timeout (milliseconds).
 * Returns true if counter reached zero, false on timeout. */
bool job_counter_wait_timeout(JobCounter *counter, uint32_t timeout_ms);

#endif /* JOB_COUNTER_H */