#pragma once

#include "std.h"

typedef struct
{
	void (*proc)(void* data);
	void* data;
} WbJobDesc;

void job_init(u32 num_worker_threads, u32 num_fibers, u32 fiber_stack_size);
void job_quit(void);