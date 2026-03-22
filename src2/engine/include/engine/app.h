#pragma once

#include <core/types.h>

typedef struct AppCallbacks
{
	int (*init)(void);
	void (*update)(double delta_time);
	void (*render)(double interpolant);
	void (*quit)(void);
} AppCallbacks;

typedef struct war_job_scheduler war_job_scheduler_t;

typedef struct app
{
    int argc;
    const char** argv;

    war_job_scheduler_t* scheduler;
} app_t;

void wc_app_init(const char* window_title, AppCallbacks callbacks);
void wc_app_quit();

bool wc_app_is_running();

void wc_app_signal_shutdown();

void wc_app_update();

int wc_app_draw();

void* wc_app_get_window_handle();

void wc_app_get_window_size(int* width, int* height);
