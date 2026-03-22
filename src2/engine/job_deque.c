/* job_deque.c - Lock-free Chase-Lev work-stealing deque implementation
 *
 * Memory ordering analysis for x86-64:
 *
 * x86-64 provides Total Store Order (TSO):
 * - Loads are not reordered with other loads
 * - Stores are not reordered with other stores
 * - Stores are not reordered with earlier loads
 * - BUT: Loads CAN be reordered with earlier stores to different locations
 *
 * This means we need explicit barriers in specific places:
 * 1. push: store to buffer THEN store to bottom (release semantics)
 * 2. pop: load bottom, store bottom, BARRIER, load top
 * 3. steal: load top, BARRIER, load bottom (acquire semantics)
 *
 * The "ABORT" result handles the race between pop and steal when there's
 * exactly one item. Rather than complex synchronization, we simply retry.
 */

#include "job_deque.h"

/*============================================================================
 * Internal Helpers
 *============================================================================*/

/* Mask for wrapping indices into buffer range */
#define DEQUE_MASK (JOB_DEQUE_CAPACITY - 1)

JOB_INLINE int64_t deque_index(int64_t i) {
    return i & DEQUE_MASK;
}

/*============================================================================
 * Implementation
 *============================================================================*/

void job_deque_init(JobDeque *dq) {
    assert(dq != NULL);
    dq->bottom = 0;
    dq->top = 0;
    memset(dq->buffer, 0, sizeof(dq->buffer));
}

JobDequeResult job_deque_push(JobDeque *dq, Job *job) {
    assert(dq != NULL);
    assert(job != NULL);

    int64_t b = job_atomic_load_relaxed(&dq->bottom);
    int64_t t = job_atomic_load_acquire(&dq->top);

    /* Check capacity */
    int64_t size = b - t;
    if (size >= JOB_DEQUE_CAPACITY) {
        return JOB_DEQUE_FULL;
    }

    /* Store job into buffer */
    dq->buffer[deque_index(b)] = job;

    /* Release barrier: ensure job is visible before we publish the new bottom.
     * On x86-64, stores are not reordered with stores, but we need a compiler
     * barrier to prevent the compiler from reordering. */
    job_compiler_barrier();

    /* Publish new bottom */
    job_atomic_store_relaxed(&dq->bottom, b + 1);

    return JOB_DEQUE_SUCCESS;
}

JobDequeResult job_deque_pop(JobDeque *dq, Job **out_job) {
    assert(dq != NULL);
    assert(out_job != NULL);

    int64_t b = job_atomic_load_relaxed(&dq->bottom);
    b = b - 1;

    /* Speculatively decrement bottom */
    job_atomic_store_relaxed(&dq->bottom, b);

    /* CRITICAL: Full memory barrier here.
     * We need to ensure:
     * 1. The store to bottom is visible to thieves before we read top
     * 2. We see any concurrent stores to top from thieves
     * On x86-64, we need mfence because loads can pass earlier stores. */
    job_memory_barrier();

    int64_t t = job_atomic_load_relaxed(&dq->top);

    if (t > b) {
        /* Deque was already empty, restore bottom */
        job_atomic_store_relaxed(&dq->bottom, b + 1);
        return JOB_DEQUE_EMPTY;
    }

    /* At least one item in deque */
    Job *job = dq->buffer[deque_index(b)];

    if (t < b) {
        /* More than one item, no contention possible with thieves */
        *out_job = job;
        return JOB_DEQUE_SUCCESS;
    }

    /* Exactly one item (t == b), race with potential thief.
     * Try to claim it by incrementing top. */
    if (!job_atomic_cas64(&dq->top, t, t + 1)) {
        /* Lost race to a thief, deque is now empty */
        job_atomic_store_relaxed(&dq->bottom, b + 1);
        return JOB_DEQUE_ABORT;
    }

    /* Won the race, got the last item */
    job_atomic_store_relaxed(&dq->bottom, b + 1);
    *out_job = job;
    return JOB_DEQUE_SUCCESS;
}

JobDequeResult job_deque_steal(JobDeque *dq, Job **out_job) {
    assert(dq != NULL);
    assert(out_job != NULL);

    /* Load top first (with acquire semantics for subsequent loads) */
    int64_t t = job_atomic_load_acquire(&dq->top);

    /* Memory barrier to ensure we see consistent bottom.
     * On x86-64, loads are not reordered, so compiler barrier suffices. */
    job_compiler_barrier();

    int64_t b = job_atomic_load_acquire(&dq->bottom);

    if (t >= b) {
        /* Empty deque */
        return JOB_DEQUE_EMPTY;
    }

    /* Non-empty, try to steal from top */
    Job *job = dq->buffer[deque_index(t)];

    /* Try to increment top to claim the job */
    if (!job_atomic_cas64(&dq->top, t, t + 1)) {
        /* Another thief or the owner got it first */
        return JOB_DEQUE_ABORT;
    }

    *out_job = job;
    return JOB_DEQUE_SUCCESS;
}

bool job_deque_is_empty(JobDeque *dq) {
    int64_t b = job_atomic_load_relaxed(&dq->bottom);
    int64_t t = job_atomic_load_relaxed(&dq->top);
    return t >= b;
}

int64_t job_deque_size(JobDeque *dq) {
    int64_t b = job_atomic_load_relaxed(&dq->bottom);
    int64_t t = job_atomic_load_relaxed(&dq->top);
    int64_t size = b - t;
    return size > 0 ? size : 0;
}