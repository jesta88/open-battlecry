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
	uint32_t num_worker_threads;
	wb_thread_t worker_threads[MAX_WORKER_THREADS];

	uint32_t num_fibers;
} WbJobSystem;

static WbJobSystem job_system;

static void fiber_proc(void* data)
{
	
}

static void worker_thread_proc(void* data)
{
	
}

void job_init(uint32_t num_worker_threads, uint32_t num_fibers, uint32_t fiber_stack_size)
{
	assert(num_worker_threads < MAX_WORKER_THREADS);
	assert(num_fibers >= 2 && (num_fibers & (num_fibers - 1)) == 0 && num_fibers < MAX_FIBERS);

	job_system.num_worker_threads = num_worker_threads;
	job_system.num_fibers = num_fibers;
}

void job_quit(void)
{
	return;
}
