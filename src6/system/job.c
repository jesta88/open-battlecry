#include "job.h"
#include "thread.h"
#include <assert.h>

enum
{
	MAX_WORKER_THREADS = 128,
	MAX_FIBERS = 256,
	MAX_JOBS = 4096
};

typedef struct
{
	u32 num_worker_threads;
	wb_thread worker_threads[MAX_WORKER_THREADS];

	u32 num_fibers;
} wb_job_scheduler_state;

static wb_job_scheduler_state job_scheduler_state;

static void fiber_proc(void* data)
{
	
}

static void worker_thread_proc(void* data)
{
	
}

void job_init(u32 num_worker_threads, u32 num_fibers, u32 fiber_stack_size)
{
	assert(num_worker_threads < MAX_WORKER_THREADS);
	assert(num_fibers >= 2 && (num_fibers & (num_fibers - 1)) == 0 && num_fibers < MAX_FIBERS);

	job_scheduler_state.num_worker_threads = num_worker_threads;
	job_scheduler_state.num_fibers = num_fibers;
}

void job_quit(void)
{
	return;
}
