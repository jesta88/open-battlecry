#pragma once

#include "types.h"

typedef struct wk_mutex wk_mutex;
typedef struct wk_thread wk_thread;
typedef struct wk_condition wk_condition;
typedef struct wk_semaphore wk_sempahore;

typedef void (*wk_thread_func)(void* arg);

uint32 wk_get_core_count(void);

wk_mutex* wk_create_mutex(void);
void wk_destroy_mutex(wk_mutex* mutex);
void wk_lock_mutex(wk_mutex* mutex);
void wk_unlock_mutex(wk_mutex* mutex);

void wk_init_thread(const char* name, wk_thread_func func, wk_thread* thread);
void wk_quit_thread(wk_thread* thread);