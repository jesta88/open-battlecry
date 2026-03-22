/* job_system.c - Main job system implementation
 *
 * Worker threads:
 * - Each worker has its own deque for jobs
 * - Workers process their own jobs first (LIFO for locality)
 * - When idle, workers steal from random victims (FIFO for fairness)
 * - Workers spin briefly then sleep on a semaphore when no work available
 *
 * Job lifecycle:
 * 1. Caller fills JobDecl and calls job_submit()
 * 2. System allocates Job from per-thread pool, copies data
 * 3. If dependency exists and isn't ready, job goes to waiting list
 * 4. Ready jobs are pushed to the submitting thread's deque
 * 5. Worker pops/steals job, executes it, decrements completion counter
 * 6. Job memory is recycled
 *
 * Priority handling:
 * - Each worker has 3 deques (one per priority level)
 * - Pop checks high priority first, then normal, then low
 * - Stealing ignores priority (takes whatever's available)
 */

#include "job_counter.h"
#include "job_deque.h"
#include "job_internal.h"

#include <engine/job.h>

/*============================================================================
 * Configuration
 *============================================================================*/

#define JOB_POOL_SIZE_PER_THREAD (JOB_QUEUE_CAPACITY * 2)
#define JOB_SPIN_COUNT_BEFORE_SLEEP 1000
#define JOB_STEAL_ATTEMPTS 3

/*============================================================================
 * Per-Thread State
 *============================================================================*/

typedef struct JOB_ALIGNAS(JOB_CACHE_LINE) WorkerThread
{
    /* Work-stealing deques (one per priority) */
    JobDeque deques[JOB_PRIORITY_COUNT];

    /* Per-thread job pool (lock-free stack) */
    JOB_ALIGNAS(JOB_CACHE_LINE) Job* job_pool;
    JOB_ALIGNAS(JOB_CACHE_LINE) volatile int64_t pool_head; /* Freelist with tag */
    uint32_t pool_capacity;

    /* Thread identification */
    uint32_t index; /* 0 = main thread */
    HANDLE thread_handle;
    DWORD thread_id;

    /* Wakeup synchronization */
    HANDLE wake_semaphore;
    volatile bool should_exit;

    /* Statistics */
    JOB_ALIGNAS(JOB_CACHE_LINE) JobStats stats;

    /* RNG state for stealing victim selection */
    uint32_t rng_state;

    /* Padding to ensure each worker is on separate cache lines */
    uint8_t _pad[JOB_CACHE_LINE];
} WorkerThread;

/*============================================================================
 * Global State
 *============================================================================*/

static struct
{
    WorkerThread* workers;  /* Array of worker threads */
    uint32_t worker_count;  /* Number of worker threads (excl main) */
    uint32_t total_threads; /* worker_count + 1 (main thread) */

    volatile bool initialized;
    volatile bool shutting_down;

    /* Global wakeup for all workers */
    HANDLE global_wake_event;
} g_job_system;

/* Thread-local worker index */
static JOB_THREAD_LOCAL uint32_t tls_thread_index = UINT32_MAX;
static JOB_THREAD_LOCAL WorkerThread* tls_worker = NULL;

/*============================================================================
 * Job Pool Management (Per-Thread)
 *============================================================================*/

/* Initialize per-thread job pool */
static bool worker_pool_init(WorkerThread* worker)
{
    worker->pool_capacity = JOB_POOL_SIZE_PER_THREAD;

    /* Allocate aligned job storage */
    size_t size = worker->pool_capacity * sizeof(Job);
    worker->job_pool = (Job*) _aligned_malloc(size, JOB_CACHE_LINE);
    if (worker->job_pool == NULL)
    {
        return false;
    }
    memset(worker->job_pool, 0, size);

    /* Build freelist (using first 8 bytes of each job as next pointer) */
    for (uint32_t i = 0; i < worker->pool_capacity - 1; ++i)
    {
        *(Job**) &worker->job_pool[i] = &worker->job_pool[i + 1];
    }
    *(Job**) &worker->job_pool[worker->pool_capacity - 1] = NULL;

    worker->pool_head = 0; /* Index 0, tag 0 */

    return true;
}

static void worker_pool_shutdown(WorkerThread* worker)
{
    if (worker->job_pool)
    {
        _aligned_free(worker->job_pool);
        worker->job_pool = NULL;
    }
}

/* Allocate job from per-thread pool (lock-free) */
static Job* job_alloc(WorkerThread* worker)
{
    /* Unpack head: low 32 bits = index, high 32 bits = tag */
    int64_t old_head, new_head;
    Job* job;

    do
    {
        old_head = job_atomic_load_acquire(&worker->pool_head);
        uint32_t idx = (uint32_t) (old_head & 0xFFFFFFFF);
        uint32_t tag = (uint32_t) (old_head >> 32);

        if (idx >= worker->pool_capacity)
        {
            return NULL; /* Pool exhausted */
        }

        job = &worker->job_pool[idx];
        Job* next = *(Job**) job;

        uint32_t next_idx = next ? (uint32_t) (next - worker->job_pool) : worker->pool_capacity;
        new_head = ((int64_t) (tag + 1) << 32) | next_idx;

    } while (!job_atomic_cas64(&worker->pool_head, old_head, new_head));

    return job;
}

/* Return job to per-thread pool (lock-free) */
static void job_free(WorkerThread* worker, Job* job)
{
    int64_t old_head, new_head;
    uint32_t job_idx = (uint32_t) (job - worker->job_pool);

    do
    {
        old_head = job_atomic_load_acquire(&worker->pool_head);
        uint32_t head_idx = (uint32_t) (old_head & 0xFFFFFFFF);
        uint32_t tag = (uint32_t) (old_head >> 32);

        /* Point job's next to current head */
        if (head_idx < worker->pool_capacity)
        {
            *(Job**) job = &worker->job_pool[head_idx];
        }
        else
        {
            *(Job**) job = NULL;
        }

        new_head = ((int64_t) (tag + 1) << 32) | job_idx;

    } while (!job_atomic_cas64(&worker->pool_head, old_head, new_head));
}

/*============================================================================
 * Worker Thread
 *============================================================================*/

/* Try to get a job from own deques (highest priority first) */
static Job* worker_pop_local(WorkerThread* worker)
{
    Job* job = NULL;

    for (int p = JOB_PRIORITY_COUNT - 1; p >= 0; --p)
    {
        JobDequeResult result = job_deque_pop(&worker->deques[p], &job);
        if (result == JOB_DEQUE_SUCCESS)
        {
            return job;
        }
        /* ABORT means retry, but we'll try other priorities first */
    }

    return NULL;
}

/* Try to steal a job from another worker */
static Job* worker_steal(WorkerThread* worker)
{
    if (g_job_system.total_threads <= 1)
    {
        return NULL;
    }

    uint32_t victim_idx = job_random_next(&worker->rng_state) % g_job_system.total_threads;

    /* Don't steal from self */
    if (victim_idx == worker->index)
    {
        victim_idx = (victim_idx + 1) % g_job_system.total_threads;
    }

    WorkerThread* victim = &g_job_system.workers[victim_idx];
    Job* job = NULL;

    /* Try all priority levels */
    for (int p = 0; p < JOB_PRIORITY_COUNT; ++p)
    {
        JobDequeResult result = job_deque_steal(&victim->deques[p], &job);
        if (result == JOB_DEQUE_SUCCESS)
        {
            worker->stats.jobs_stolen++;
            return job;
        }
    }

    worker->stats.steal_attempts++;
    return NULL;
}

/* Execute a single job */
static void worker_execute_job(WorkerThread* worker, Job* job)
{
    assert(job != NULL);
    assert(job->func != NULL);

    /* Execute the job function */
    job->func(job->payload);

    worker->stats.jobs_executed++;

    /* Decrement completion counter if present */
    if (job->completion)
    {
        job_counter_decrement(job->completion);
    }

    /* Return job to pool */
    job_free(worker, job);
}

/* Worker thread main loop */
static DWORD WINAPI worker_thread_proc(LPVOID param)
{
    WorkerThread* worker = (WorkerThread*) param;

    /* Set thread-local state */
    tls_thread_index = worker->index;
    tls_worker = worker;

    /* Set thread affinity to specific core */
    if (worker->index < 64)
    {
        SetThreadAffinityMask(GetCurrentThread(), 1ULL << worker->index);
    }

    uint32_t spin_count = 0;

    while (!worker->should_exit)
    {
        Job* job = NULL;

        /* Try local queue first */
        job = worker_pop_local(worker);

        /* If no local work, try stealing */
        if (job == NULL)
        {
            for (int i = 0; i < JOB_STEAL_ATTEMPTS && job == NULL; ++i)
            {
                job = worker_steal(worker);
            }
        }

        if (job != NULL)
        {
            worker_execute_job(worker, job);
            spin_count = 0;
        }
        else
        {
            /* No work available */
            spin_count++;
            worker->stats.spin_cycles++;

            if (spin_count < JOB_SPIN_COUNT_BEFORE_SLEEP)
            {
                /* Spin with pause instruction */
                YieldProcessor();
            }
            else
            {
                /* Sleep on semaphore */
                spin_count = 0;
                WaitForSingleObject(worker->wake_semaphore, 1); /* 1ms timeout */
            }
        }
    }

    return 0;
}

/* Wake up workers to process new jobs */
static void wake_workers(uint32_t count)
{
    for (uint32_t i = 1; i <= count && i <= g_job_system.worker_count; ++i)
    {
        ReleaseSemaphore(g_job_system.workers[i].wake_semaphore, 1, NULL);
    }
}

/*============================================================================
 * Initialization / Shutdown
 *============================================================================*/

bool job_system_init(uint32_t worker_count)
{
    if (g_job_system.initialized)
    {
        return true;
    }

    /* Auto-detect worker count */
    if (worker_count == 0)
    {
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        worker_count = sysinfo.dwNumberOfProcessors;
        if (worker_count > 1)
        {
            worker_count--; /* Reserve one core for main thread */
        }
    }

    if (worker_count > JOB_MAX_WORKERS)
    {
        worker_count = JOB_MAX_WORKERS;
    }

    g_job_system.worker_count = worker_count;
    g_job_system.total_threads = worker_count + 1;

    /* Initialize counter pool */
    if (!job_counter_pool_init())
    {
        return false;
    }

    /* Allocate worker array (includes slot 0 for main thread) */
    size_t workers_size = g_job_system.total_threads * sizeof(WorkerThread);
    g_job_system.workers = (WorkerThread*) _aligned_malloc(workers_size, JOB_CACHE_LINE);
    if (g_job_system.workers == NULL)
    {
        job_counter_pool_shutdown();
        return false;
    }
    memset(g_job_system.workers, 0, workers_size);

    /* Create global wake event */
    g_job_system.global_wake_event = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (g_job_system.global_wake_event == NULL)
    {
        _aligned_free(g_job_system.workers);
        job_counter_pool_shutdown();
        return false;
    }

    /* Initialize all workers (including main thread at index 0) */
    for (uint32_t i = 0; i < g_job_system.total_threads; ++i)
    {
        WorkerThread* worker = &g_job_system.workers[i];
        worker->index = i;
        worker->rng_state = i + 1; /* Seed RNG with thread index */
        worker->should_exit = false;

        /* Initialize deques */
        for (int p = 0; p < JOB_PRIORITY_COUNT; ++p)
        {
            job_deque_init(&worker->deques[p]);
        }

        /* Initialize job pool */
        if (!worker_pool_init(worker))
        {
            /* Cleanup and fail */
            for (uint32_t j = 0; j < i; ++j)
            {
                worker_pool_shutdown(&g_job_system.workers[j]);
            }
            CloseHandle(g_job_system.global_wake_event);
            _aligned_free(g_job_system.workers);
            job_counter_pool_shutdown();
            return false;
        }

        /* Create wake semaphore for worker threads */
        if (i > 0)
        {
            worker->wake_semaphore = CreateSemaphoreW(NULL, 0, INT32_MAX, NULL);
            if (worker->wake_semaphore == NULL)
            {
                for (uint32_t j = 0; j <= i; ++j)
                {
                    worker_pool_shutdown(&g_job_system.workers[j]);
                    if (j > 0 && g_job_system.workers[j].wake_semaphore)
                    {
                        CloseHandle(g_job_system.workers[j].wake_semaphore);
                    }
                }
                CloseHandle(g_job_system.global_wake_event);
                _aligned_free(g_job_system.workers);
                job_counter_pool_shutdown();
                return false;
            }
        }
    }

    /* Set main thread TLS */
    tls_thread_index = 0;
    tls_worker = &g_job_system.workers[0];

    /* Start worker threads */
    for (uint32_t i = 1; i < g_job_system.total_threads; ++i)
    {
        WorkerThread* worker = &g_job_system.workers[i];
        worker->thread_handle = CreateThread(NULL, 0, worker_thread_proc, worker, 0, &worker->thread_id);

        if (worker->thread_handle == NULL)
        {
            /* Signal existing workers to exit and cleanup */
            g_job_system.shutting_down = true;
            for (uint32_t j = 1; j < i; ++j)
            {
                g_job_system.workers[j].should_exit = true;
                ReleaseSemaphore(g_job_system.workers[j].wake_semaphore, 1, NULL);
            }
            for (uint32_t j = 1; j < i; ++j)
            {
                WaitForSingleObject(g_job_system.workers[j].thread_handle, INFINITE);
                CloseHandle(g_job_system.workers[j].thread_handle);
            }
            for (uint32_t j = 0; j < g_job_system.total_threads; ++j)
            {
                worker_pool_shutdown(&g_job_system.workers[j]);
                if (j > 0 && g_job_system.workers[j].wake_semaphore)
                {
                    CloseHandle(g_job_system.workers[j].wake_semaphore);
                }
            }
            CloseHandle(g_job_system.global_wake_event);
            _aligned_free(g_job_system.workers);
            job_counter_pool_shutdown();
            return false;
        }

        /* Set thread priority to slightly above normal for responsiveness */
        SetThreadPriority(worker->thread_handle, THREAD_PRIORITY_ABOVE_NORMAL);
    }

    g_job_system.initialized = true;
    return true;
}

void job_system_shutdown(void)
{
    if (!g_job_system.initialized)
    {
        return;
    }

    g_job_system.shutting_down = true;

    /* Signal all workers to exit */
    for (uint32_t i = 1; i < g_job_system.total_threads; ++i)
    {
        g_job_system.workers[i].should_exit = true;
        ReleaseSemaphore(g_job_system.workers[i].wake_semaphore, 1, NULL);
    }

    /* Wait for all workers to finish */
    for (uint32_t i = 1; i < g_job_system.total_threads; ++i)
    {
        WaitForSingleObject(g_job_system.workers[i].thread_handle, INFINITE);
        CloseHandle(g_job_system.workers[i].thread_handle);
        CloseHandle(g_job_system.workers[i].wake_semaphore);
    }

    /* Cleanup pools */
    for (uint32_t i = 0; i < g_job_system.total_threads; ++i)
    {
        worker_pool_shutdown(&g_job_system.workers[i]);
    }

    CloseHandle(g_job_system.global_wake_event);
    _aligned_free(g_job_system.workers);
    g_job_system.workers = NULL;

    job_counter_pool_shutdown();

    g_job_system.initialized = false;
    g_job_system.shutting_down = false;
}

uint32_t job_system_worker_count(void)
{
    return g_job_system.worker_count;
}

uint32_t job_system_thread_index(void)
{
    return tls_thread_index;
}

/*============================================================================
 * Job Submission
 *============================================================================*/

static bool submit_job_internal(const JobDecl* decl, WorkerThread* worker)
{
    /* Allocate job from pool */
    Job* job = job_alloc(worker);
    if (job == NULL)
    {
        return false;
    }

    /* Setup job */
    job->func = decl->func;
    job->completion = decl->completion;

    /* Copy data into payload or store pointer */
    if (decl->data != NULL)
    {
        if (decl->data_size > 0 && decl->data_size <= JOB_PAYLOAD_SIZE)
        {
            memcpy(job->payload, decl->data, decl->data_size);
        }
        else
        {
            /* Store pointer in payload */
            *(void**) job->payload = decl->data;
        }
    }

    /* Handle dependency */
    if (decl->dependency != NULL && !job_counter_is_zero(decl->dependency))
    {
        /* TODO: Implement waiting job queue for deferred execution
         * For now, we spin-wait (not ideal but functional) */
        while (!job_counter_is_zero(decl->dependency))
        {
            /* Help process other jobs while waiting */
            Job* other = worker_pop_local(worker);
            if (other)
            {
                worker_execute_job(worker, other);
            }
            else
            {
                YieldProcessor();
            }
        }
    }

    /* Push to deque */
    JobDequeResult result = job_deque_push(&worker->deques[decl->priority], job);
    if (result != JOB_DEQUE_SUCCESS)
    {
        job_free(worker, job);
        return false;
    }

    /* Wake a worker */
    wake_workers(1);

    return true;
}

bool job_submit(const JobDecl* decl)
{
    assert(g_job_system.initialized);
    assert(decl != NULL);
    assert(decl->func != NULL);

    WorkerThread* worker = tls_worker;
    if (worker == NULL)
    {
        /* Called from non-worker thread, use main thread's queue */
        worker = &g_job_system.workers[0];
    }

    return submit_job_internal(decl, worker);
}

bool job_submit_batch(const JobDecl* decls, uint32_t count)
{
    assert(g_job_system.initialized);
    assert(decls != NULL);

    WorkerThread* worker = tls_worker;
    if (worker == NULL)
    {
        worker = &g_job_system.workers[0];
    }

    for (uint32_t i = 0; i < count; ++i)
    {
        if (!submit_job_internal(&decls[i], worker))
        {
            return false;
        }
    }

    /* Wake multiple workers */
    wake_workers(count);

    return true;
}

/*============================================================================
 * Parallel For
 *============================================================================*/

typedef struct ParallelForData
{
    JobBatchFunc func;
    void* user_data;
    uint32_t begin;
    uint32_t end;
} ParallelForData;

static void parallel_for_job(void* ctx)
{
    ParallelForData* data = (ParallelForData*) ctx;
    JobRange range = {.begin = data->begin, .end = data->end, .user_data = data->user_data};
    data->func(&range);
}

JobCounter* job_parallel_for(JobBatchFunc func, void* user_data, uint32_t count, uint32_t granularity, JobCounter* dependency)
{
    assert(g_job_system.initialized);
    assert(func != NULL);

    if (count == 0)
    {
        return job_counter_alloc(0);
    }

    /* Auto-select granularity */
    if (granularity == 0)
    {
        granularity = (count + g_job_system.total_threads * 4 - 1) / (g_job_system.total_threads * 4);
        if (granularity < 1)
            granularity = 1;
        if (granularity > 256)
            granularity = 256;
    }

    /* Calculate job count */
    uint32_t job_count = (count + granularity - 1) / granularity;

    /* Allocate completion counter */
    JobCounter* completion = job_counter_alloc(job_count);
    if (completion == NULL)
    {
        return NULL;
    }

    /* Submit jobs */
    WorkerThread* worker = tls_worker;
    if (worker == NULL)
    {
        worker = &g_job_system.workers[0];
    }

    for (uint32_t i = 0; i < count; i += granularity)
    {
        ParallelForData data = {.func = func,
                                .user_data = user_data,
                                .begin = i,
                                .end = (i + granularity < count) ? i + granularity : count};

        JobDecl decl = {.func = parallel_for_job,
                        .data = &data,
                        .data_size = sizeof(data),
                        .priority = JOB_PRIORITY_NORMAL,
                        .dependency = dependency,
                        .completion = completion};

        if (!submit_job_internal(&decl, worker))
        {
            /* Failed to submit - decrement counter for unjobmitted jobs */
            for (uint32_t j = i; j < count; j += granularity)
            {
                job_counter_decrement(completion);
            }
            break;
        }
    }

    wake_workers(job_count);

    return completion;
}

/*============================================================================
 * Assist Functions
 *============================================================================*/

void job_assist_until(JobCounter* counter)
{
    WorkerThread* worker = tls_worker;
    if (worker == NULL)
    {
        worker = &g_job_system.workers[0];
    }

    while (!job_counter_is_zero(counter))
    {
        Job* job = worker_pop_local(worker);

        if (job == NULL)
        {
            job = worker_steal(worker);
        }

        if (job != NULL)
        {
            worker_execute_job(worker, job);
        }
        else
        {
            YieldProcessor();
        }
    }
}

bool job_assist_one(void)
{
    WorkerThread* worker = tls_worker;
    if (worker == NULL)
    {
        worker = &g_job_system.workers[0];
    }

    Job* job = worker_pop_local(worker);

    if (job == NULL)
    {
        job = worker_steal(worker);
    }

    if (job != NULL)
    {
        worker_execute_job(worker, job);
        return true;
    }

    return false;
}

/*============================================================================
 * Statistics
 *============================================================================*/

void job_stats_get(uint32_t thread_index, JobStats* out_stats)
{
    assert(out_stats != NULL);

    if (thread_index >= g_job_system.total_threads)
    {
        memset(out_stats, 0, sizeof(*out_stats));
        return;
    }

    *out_stats = g_job_system.workers[thread_index].stats;
}

void job_stats_reset(void)
{
    for (uint32_t i = 0; i < g_job_system.total_threads; ++i)
    {
        memset(&g_job_system.workers[i].stats, 0, sizeof(JobStats));
    }
}