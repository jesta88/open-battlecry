#pragma once

#include <stdint.h>

typedef uint32_t thread_t;
typedef uint32_t fiber_t;
typedef uint32_t mutex_t;
typedef uint32_t semaphore_t;

typedef void (*thread_function_t)(void* user_data);

//thread_t s_create_thread(thread_function_t function, void* user_data);
//void s_destroy_thread(thread_t thread);
//void s_wait_thread(thread_t thread);
//
//mutex_t s_create_critical_section(void);
//void s_destroy_critical_section(mutex_t critical_section);
//void s_lock_critical_section(mutex_t mutex);
//void s_unlock_critical_section(mutex_t mutex);
//
//condition_t s_create_condition(void);
//void s_wait_condition(condition_t condition, mutex_t mutex);
//void s_condition_wake_single(condition_t condition);
//void s_condition_wake_all(condition_t condition);
//
//uint32_t s_cpu_count(void);
//void s_sleep(uint32_t ms);