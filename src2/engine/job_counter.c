/* job_counter.c - Lock-free counter pool implementation
 *
 * The counter pool uses a lock-free freelist for allocation.
 *
 * Key invariants:
 * 1. A counter can only be recycled when value=0 AND refcount=0
 * 2. Generation increments on each recycle to prevent ABA
 * 3. The caller of job_counter_alloc implicitly holds a reference
 * 4. job_parallel_for returns a counter that the caller must wait on
 *    (waiting releases the implicit reference)
 *
 * Memory ordering:
 * - State modifications use CAS with full barrier semantics
 * - Freelist uses a simple tagged pointer with generation
 */

#include "job_counter.h"

/*============================================================================
 * Freelist
 *============================================================================*/

/* Freelist node - overlays the counter when not in use */
typedef struct FreeNode
{
    struct FreeNode* next;
    uint16_t generation; /* Preserved across recycles */
} FreeNode;

/* Freelist head with ABA tag */
typedef union FreelistHead
{
    struct
    {
        FreeNode* node;
        uint64_t tag;
    };
    LARGE_INTEGER packed; /* For 128-bit CAS */
} FreelistHead;

/*============================================================================
 * Global Pool State
 *============================================================================*/

static struct
{
    /* Pool storage */
    JOB_ALIGNAS(JOB_CACHE_LINE) JobCounter counters[JOB_COUNTER_POOL_SIZE];

    /* Lock-free freelist head (128-bit for DCAS) */
    JOB_ALIGNAS(16) volatile FreelistHead freelist;

    /* Statistics */
    volatile int32_t allocated_count;
    volatile int32_t high_water_mark;

    bool initialized;
} g_counter_pool;

/*============================================================================
 * Freelist Operations
 *============================================================================*/

JOB_INLINE bool freelist_cas(volatile FreelistHead* head, FreelistHead expected, FreelistHead desired)
{
    return _InterlockedCompareExchange128((volatile __int64*) head, (int64_t) desired.tag, (int64_t) desired.node, (__int64*) &expected) !=
           0;
}

static JobCounter* freelist_pop(void)
{
    FreelistHead head, new_head;

    for (;;)
    {
        head.node = g_counter_pool.freelist.node;
        head.tag = g_counter_pool.freelist.tag;
        job_compiler_barrier();

        if (head.node == NULL)
        {
            return NULL; /* Pool exhausted */
        }

        new_head.node = ((FreeNode*) head.node)->next;
        new_head.tag = head.tag + 1;

        if (freelist_cas(&g_counter_pool.freelist, head, new_head))
        {
            job_atomic_fetch_add32(&g_counter_pool.allocated_count, 1);

            /* Update high water mark */
            int32_t count = g_counter_pool.allocated_count;
            int32_t hwm = g_counter_pool.high_water_mark;
            while (count > hwm)
            {
                if (job_atomic_cas32(&g_counter_pool.high_water_mark, hwm, count))
                {
                    break;
                }
                hwm = g_counter_pool.high_water_mark;
            }

            return (JobCounter*) head.node;
        }
        /* CAS failed, retry */
        YieldProcessor();
    }
}

static void freelist_push(JobCounter* counter)
{
    FreelistHead head, new_head;
    FreeNode* node = (FreeNode*) counter;

    for (;;)
    {
        head.node = g_counter_pool.freelist.node;
        head.tag = g_counter_pool.freelist.tag;
        job_compiler_barrier();

        node->next = head.node;
        /* Preserve and increment generation */
        node->generation++;

        new_head.node = node;
        new_head.tag = head.tag + 1;

        if (freelist_cas(&g_counter_pool.freelist, head, new_head))
        {
            job_atomic_fetch_sub32(&g_counter_pool.allocated_count, 1);
            return;
        }
        /* CAS failed, retry */
        YieldProcessor();
    }
}

/*============================================================================
 * State Helpers
 *============================================================================*/

JOB_INLINE JobCounterState state_unpack(int64_t packed)
{
    JobCounterState s;
    s.packed = packed;
    return s;
}

JOB_INLINE int64_t state_pack(uint32_t value, uint16_t refcount, uint16_t gen)
{
    JobCounterState s;
    s.value = value;
    s.refcount = refcount;
    s.generation = gen;
    return s.packed;
}

/*============================================================================
 * Pool Management
 *============================================================================*/

bool job_counter_pool_init(void)
{
    if (g_counter_pool.initialized)
    {
        return true;
    }

    /* Initialize all counters and build freelist */
    memset(g_counter_pool.counters, 0, sizeof(g_counter_pool.counters));

    /* Create Win32 events for each counter */
    for (int i = 0; i < JOB_COUNTER_POOL_SIZE; ++i)
    {
        JobCounter* c = &g_counter_pool.counters[i];
        c->event = CreateEventW(NULL, TRUE, FALSE, NULL); /* Manual reset */
        if (c->event == NULL)
        {
            /* Cleanup on failure */
            for (int j = 0; j < i; ++j)
            {
                CloseHandle(g_counter_pool.counters[j].event);
            }
            return false;
        }
    }

    /* Build freelist (reverse order so index 0 is allocated first) */
    g_counter_pool.freelist.node = NULL;
    g_counter_pool.freelist.tag = 0;

    for (int i = JOB_COUNTER_POOL_SIZE - 1; i >= 0; --i)
    {
        FreeNode* node = (FreeNode*) &g_counter_pool.counters[i];
        node->next = g_counter_pool.freelist.node;
        node->generation = 0;
        g_counter_pool.freelist.node = node;
    }

    g_counter_pool.allocated_count = 0;
    g_counter_pool.high_water_mark = 0;
    g_counter_pool.initialized = true;

    return true;
}

void job_counter_pool_shutdown(void)
{
    if (!g_counter_pool.initialized)
    {
        return;
    }

    /* Close all Win32 events */
    for (int i = 0; i < JOB_COUNTER_POOL_SIZE; ++i)
    {
        if (g_counter_pool.counters[i].event != NULL)
        {
            CloseHandle(g_counter_pool.counters[i].event);
            g_counter_pool.counters[i].event = NULL;
        }
    }

    g_counter_pool.initialized = false;
}

/*============================================================================
 * Public API
 *============================================================================*/

JobCounter* job_counter_alloc(uint32_t initial_value)
{
    assert(g_counter_pool.initialized);

    JobCounter* counter = freelist_pop();
    if (counter == NULL)
    {
        return NULL;
    }

    /* Get preserved generation from freelist storage */
    uint16_t gen = ((FreeNode*) counter)->generation;

    /* Initialize state: value=initial, refcount=1 (caller holds ref) */
    counter->state = state_pack(initial_value, 1, gen);
    counter->waiters = 0;

    /* Reset event to non-signaled */
    ResetEvent(counter->event);

    return counter;
}

void job_counter_addref(JobCounter* counter)
{
    if (counter == NULL)
        return;

    int64_t old_state, new_state;
    JobCounterState s;

    do
    {
        old_state = job_atomic_load_acquire(&counter->state);
        s = state_unpack(old_state);
        assert(s.refcount > 0 && "addref on freed counter");
        assert(s.refcount < UINT16_MAX && "refcount overflow");
        new_state = state_pack(s.value, s.refcount + 1, s.generation);
    } while (!job_atomic_cas64(&counter->state, old_state, new_state));
}

void job_counter_release(JobCounter* counter)
{
    if (counter == NULL)
        return;

    int64_t old_state, new_state;
    JobCounterState s;

    do
    {
        old_state = job_atomic_load_acquire(&counter->state);
        s = state_unpack(old_state);
        assert(s.refcount > 0 && "release on freed counter");
        new_state = state_pack(s.value, s.refcount - 1, s.generation);
    } while (!job_atomic_cas64(&counter->state, old_state, new_state));

    /* Check if we should recycle (value=0 AND refcount=0) */
    s = state_unpack(new_state);
    if (s.value == 0 && s.refcount == 0)
    {
        freelist_push(counter);
    }
}

void job_counter_increment(JobCounter* counter, uint32_t amount)
{
    if (counter == NULL || amount == 0)
        return;

    int64_t old_state, new_state;

    do
    {
        old_state = job_atomic_load_acquire(&counter->state);
        JobCounterState s = state_unpack(old_state);
        assert(s.refcount > 0 && "increment on freed counter");
        new_state = state_pack(s.value + amount, s.refcount, s.generation);
    } while (!job_atomic_cas64(&counter->state, old_state, new_state));
}

void job_counter_decrement(JobCounter* counter)
{
    if (counter == NULL)
        return;

    int64_t old_state, new_state;
    JobCounterState s;
    bool should_signal = false;
    bool should_recycle = false;

    do
    {
        old_state = job_atomic_load_acquire(&counter->state);
        s = state_unpack(old_state);
        assert(s.value > 0 && "decrement below zero");

        uint32_t new_value = s.value - 1;
        new_state = state_pack(new_value, s.refcount, s.generation);
        should_signal = (new_value == 0);
    } while (!job_atomic_cas64(&counter->state, old_state, new_state));

    if (should_signal)
    {
        /* Wake all waiters */
        if (counter->waiters > 0)
        {
            SetEvent(counter->event);
        }

        /* Check if we should recycle (no more references) */
        s = state_unpack(new_state);
        if (s.refcount == 0)
        {
            freelist_push(counter);
        }
    }
}

bool job_counter_is_zero(const JobCounter* counter)
{
    if (counter == NULL)
        return true;

    JobCounterState s = state_unpack(job_atomic_load_acquire((volatile int64_t*) &counter->state));
    return s.value == 0;
}

uint32_t job_counter_value(const JobCounter* counter)
{
    if (counter == NULL)
        return 0;

    JobCounterState s = state_unpack(job_atomic_load_acquire((volatile int64_t*) &counter->state));
    return s.value;
}

void job_counter_wait(JobCounter* counter)
{
    job_counter_wait_timeout(counter, INFINITE);
}

bool job_counter_wait_timeout(JobCounter* counter, uint32_t timeout_ms)
{
    if (counter == NULL)
        return true;

    /* Fast path: already zero */
    if (job_counter_is_zero(counter))
    {
        job_counter_release(counter);
        return true;
    }

    /* Register as waiter */
    job_atomic_increment32(&counter->waiters);

    /* Check again after registering (avoid missed wakeup) */
    if (job_counter_is_zero(counter))
    {
        job_atomic_decrement32(&counter->waiters);
        job_counter_release(counter);
        return true;
    }

    /* Block on event */
    DWORD result = WaitForSingleObject(counter->event, timeout_ms);

    job_atomic_decrement32(&counter->waiters);

    bool completed = (result == WAIT_OBJECT_0) || job_counter_is_zero(counter);

    if (completed)
    {
        job_counter_release(counter);
    }

    return completed;
}