/* job_deque.h - Lock-free Chase-Lev work-stealing deque
 *
 * Implementation based on:
 * "Dynamic Circular Work-Stealing Deque" by Chase & Lev (SPAA 2005)
 * With simplifications from "Correct and Efficient Work-Stealing for Weak
 * Memory Models" by Le et al. (PPoPP 2013)
 *
 * Properties:
 * - Single producer (owner thread): push/pop from bottom
 * - Multiple consumers (thieves): steal from top
 * - Fixed capacity (no dynamic resizing - keeps implementation simple)
 * - LIFO for owner (cache locality), FIFO for thieves (fairness)
 */

#ifndef JOB_DEQUE_H
#define JOB_DEQUE_H

#include "job_internal.h"

/*============================================================================
 * Configuration
 *============================================================================*/

#ifndef JOB_DEQUE_CAPACITY
#define JOB_DEQUE_CAPACITY 4096  /* Must be power of 2 */
#endif

_Static_assert(JOB_DEQUE_CAPACITY > 0 &&
               (JOB_DEQUE_CAPACITY & (JOB_DEQUE_CAPACITY - 1)) == 0,
               "JOB_DEQUE_CAPACITY must be a power of 2");

/*============================================================================
 * Types
 *============================================================================*/

/* Result codes for deque operations */
typedef enum JobDequeResult {
    JOB_DEQUE_SUCCESS = 0,      /* Operation succeeded */
    JOB_DEQUE_EMPTY,            /* Deque was empty */
    JOB_DEQUE_ABORT,            /* CAS failed due to contention (retry) */
    JOB_DEQUE_FULL              /* Deque is at capacity */
} JobDequeResult;

/*
 * Work-stealing deque structure
 *
 * Layout optimized to minimize false sharing:
 * - bottom: written only by owner, read by thieves
 * - top: written by thieves (CAS), read by owner and thieves
 * - buffer: read/written by both, but at different indices
 */
typedef struct JOB_ALIGNAS(JOB_CACHE_LINE) JobDeque {
    /* Bottom index - modified only by owner thread */
    JOB_ALIGNAS(JOB_CACHE_LINE) volatile int64_t bottom;

    /* Top index - modified by thieves via CAS */
    JOB_ALIGNAS(JOB_CACHE_LINE) volatile int64_t top;

    /* Circular buffer of job pointers */
    JOB_ALIGNAS(JOB_CACHE_LINE) Job *buffer[JOB_DEQUE_CAPACITY];
} JobDeque;

/*============================================================================
 * API
 *============================================================================*/

/* Initialize a deque. Must be called before use. */
void job_deque_init(JobDeque *dq);

/* Push a job onto the bottom of the deque (owner only).
 * Returns JOB_DEQUE_FULL if capacity exceeded. */
JobDequeResult job_deque_push(JobDeque *dq, Job *job);

/* Pop a job from the bottom of the deque (owner only).
 * Returns JOB_DEQUE_EMPTY if no jobs available.
 * May return JOB_DEQUE_ABORT on contention with a thief - caller should retry. */
JobDequeResult job_deque_pop(JobDeque *dq, Job **out_job);

/* Steal a job from the top of the deque (any thread).
 * Returns JOB_DEQUE_EMPTY if no jobs available.
 * Returns JOB_DEQUE_ABORT on contention - caller should try another victim. */
JobDequeResult job_deque_steal(JobDeque *dq, Job **out_job);

/* Check if deque is empty (approximate, may race with push/pop/steal) */
bool job_deque_is_empty(JobDeque *dq);

/* Get approximate size (may race with concurrent operations) */
int64_t job_deque_size(JobDeque *dq);

#endif /* JOB_DEQUE_H */