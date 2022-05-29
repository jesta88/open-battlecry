#pragma once

#include "types.h"

typedef struct
{
	void (*proc)(void* data);
	void* data;
} WbJobDesc;

void job_init(uint32_t num_worker_threads, uint32_t num_fibers, uint32_t fiber_stack_size);
void job_quit(void);