#pragma once

#include "types.h"

typedef struct { u64 handle; } wb_fiber;
typedef struct { u64 handle; } wb_mutex;
typedef struct { u64 handle[2]; } wb_thread;
typedef struct { u64 handle; } wb_condition;

typedef u32 (*wb_thread_entry)(void*);

wb_thread os_create_thread(wb_thread_entry thread_entry, void* data);
void os_destroy_thread(wb_thread thread);
void os_wait_thread(wb_thread thread);

wb_mutex os_create_mutex(void);
void os_destroy_mutex(wb_mutex mutex);
void os_lock_mutex(wb_mutex mutex);
void os_unlock_mutex(wb_mutex mutex);

wb_condition os_create_condition(void);
void os_wait_condition(wb_condition condition, wb_mutex mutex);
void os_condition_wake_single(wb_condition condition);
void os_condition_wake_all(wb_condition condition);

u32 os_cpu_count(void);
void os_sleep(u32 ms);