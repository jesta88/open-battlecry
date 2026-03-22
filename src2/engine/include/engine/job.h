/* job_system.h - High-performance work-stealing job system
 *
 * Architecture: Lock-free work-stealing deques with atomic counter dependencies
 * Target: Win32, C17, x64
 *
 * Features:
 * - Lock-free job submission and execution
 * - Work-stealing for automatic load balancing
 * - Counter-based dependencies for arbitrary DAGs
 * - Parallel-for helper for data-parallel workloads
 * - Cache-line aligned jobs for optimal memory access
 *
 * Usage:
 *   1. Call job_system_init() at startup
 *   2. Submit jobs with job_submit() or job_parallel_for()
 *   3. Wait on completion counters with job_counter_wait()
 *   4. Call job_system_shutdown() at exit
 */

#ifndef JOB_SYSTEM_H
#define JOB_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>

/*============================================================================
 * Configuration
 *============================================================================*/

#define JOB_MAX_WORKERS 64
#define JOB_QUEUE_CAPACITY 4096 /* Per-thread, must be power of 2 */

/*============================================================================
 * Types
 *============================================================================*/

/* Job function signature. Context points to embedded or external data. */
typedef void (*JobFunc)(void* ctx);

/* Opaque counter handle for dependencies and completion tracking.
 * Counters are allocated from a pool and recycled automatically. */
typedef struct JobCounter JobCounter;

/* Job priority levels. Higher priority jobs are dequeued first locally. */
typedef enum JobPriority
{
    JOB_PRIORITY_LOW = 0,
    JOB_PRIORITY_NORMAL = 1,
    JOB_PRIORITY_HIGH = 2,
    JOB_PRIORITY_COUNT
} JobPriority;

/* Job declaration - filled by caller, submitted to system */
typedef struct JobDecl
{
    JobFunc func;           /* Required: function to execute */
    void* data;             /* Optional: if NULL, uses embedded storage */
    uint32_t data_size;     /* Size to copy into embedded storage (max JOB_PAYLOAD_SIZE) */
    JobPriority priority;   /* Scheduling priority */
    JobCounter* dependency; /* Wait for this counter to hit 0 before running */
    JobCounter* completion; /* Decrement this counter when job finishes */
} JobDecl;

/* Parallel-for range passed to batch job functions */
typedef struct JobRange
{
    uint32_t begin;  /* Inclusive start index */
    uint32_t end;    /* Exclusive end index */
    void* user_data; /* User context pointer */
} JobRange;

typedef void (*JobBatchFunc)(const JobRange* range);

/* System statistics for profiling */
typedef struct JobStats
{
    uint64_t jobs_executed;  /* Total jobs run on this thread */
    uint64_t jobs_stolen;    /* Jobs stolen from other threads */
    uint64_t steal_attempts; /* Total steal attempts */
    uint64_t spin_cycles;    /* Cycles spent spinning */
} JobStats;

/*============================================================================
 * Initialization / Shutdown
 *============================================================================*/

/* Initialize job system.
 * worker_count: 0 = auto-detect (physical cores - 1 for main thread)
 * Returns false on failure. */
bool job_system_init(uint32_t worker_count);

/* Shutdown and wait for all workers to terminate.
 * All pending jobs are completed before return. */
void job_system_shutdown(void);

/* Query worker count (excluding main thread) */
uint32_t job_system_worker_count(void);

/* Get current thread's worker index (0 = main thread, 1..N = workers) */
uint32_t job_system_thread_index(void);

/*============================================================================
 * Counter Management
 *============================================================================*/

/* Allocate a counter initialized to `initial_value`.
 * Counters are recycled when they reach 0 and no jobs reference them.
 * Returns NULL if pool exhausted. */
JobCounter* job_counter_alloc(uint32_t initial_value);

/* Manually increment counter (for dynamic job spawning patterns) */
void job_counter_increment(JobCounter* counter, uint32_t amount);

/* Check if counter has reached zero (non-blocking) */
bool job_counter_is_zero(const JobCounter* counter);

/* Block until counter reaches zero. Assists with job execution while waiting.
 * Safe to call from any thread including workers.
 * Releases the caller's reference to the counter. */
void job_counter_wait(JobCounter* counter);

/* Block with timeout (milliseconds). Returns true if counter reached zero.
 * Only releases reference if counter completed. */
bool job_counter_wait_timeout(JobCounter* counter, uint32_t timeout_ms);

/*============================================================================
 * Job Submission
 *============================================================================*/

/* Submit a single job. Returns false if queue is full.
 * If decl->completion is NULL, job runs as fire-and-forget. */
bool job_submit(const JobDecl* decl);

/* Submit multiple jobs atomically. All share the same dependency/completion.
 * More efficient than individual submissions. */
bool job_submit_batch(const JobDecl* decls, uint32_t count);

/* Parallel-for helper: splits [0, count) into jobs across all workers.
 * `granularity`: items per job (0 = auto-select based on count/workers)
 * `dependency`: wait for this before starting (can be NULL)
 * Returns completion counter (caller must wait on it to release). */
JobCounter* job_parallel_for(JobBatchFunc func, void* user_data, uint32_t count, uint32_t granularity, JobCounter* dependency);

/*============================================================================
 * Main Thread Helpers
 *============================================================================*/

/* Process jobs on the calling thread until counter reaches zero.
 * Use this instead of busy-waiting to make progress on the main thread.
 * Does NOT release the counter reference (unlike job_counter_wait). */
void job_assist_until(JobCounter* counter);

/* Process a single job if one is available. Returns true if work was done.
 * Useful for frame pacing or interleaving with main-thread work. */
bool job_assist_one(void);

/*============================================================================
 * Profiling
 *============================================================================*/

/* Get statistics for a specific worker (0 = main thread) */
void job_stats_get(uint32_t thread_index, JobStats* out_stats);

/* Reset all statistics */
void job_stats_reset(void);

#endif /* JOB_SYSTEM_H */