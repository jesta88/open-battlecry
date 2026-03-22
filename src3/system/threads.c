#include "../threads.h"
#include "../defines.h"
#include "../debug.h"
#include "../time_internal.h"
#include "windows_misc.h"
#include "windows_atomic.h"
#include "windows_threads.h"
#include "windows_sysinfo.h"
#include <stdlib.h>
#include <intrin.h>
#include <string.h>

#define WK_DEFAULT_SPINCOUNT      1500
#define WK_DEFAULT_STACK_SIZE     1 << 16
#define WK_MAX_THREADS            32
#define WK_MAX_THREAD_NAME_LENGTH 32

enum wk_thread_priority
{
    WK_THREAD_PRIORITY_NORMAL = 0,
    WK_THREAD_PRIORITY_HIGH = 2
};

enum wk_thread_state
{
    WK_THREAD_STATE_READY,
    WK_THREAD_STATE_RUNNING,
    WK_THREAD_STATE_TERMINATING,
    WK_THREAD_STATE_IDLE
};

struct wk_mutex
{
    CRITICAL_SECTION cs;
};

struct wk_thread
{
    volatile long state;
    int priority;
    usize stack_size;
    uptr id;
    wk_thread_func func;
#ifndef WK_RELEASE
    char name[WK_MAX_THREAD_NAME_LENGTH];
#endif
};

struct wk_semaphore
{
    uptr handle;
    //_Atomic(u32) count;
};

static HANDLE main_thread_handle;
static uint32 main_thread_id = WK_INVALID_ID_U32;
static wk_mutex thread_mutex;

static _Thread_local bool tls_is_main_thread = false;
#ifdef WK_DEBUG
static _Thread_local const char* tls_thread_name = NULL;
#endif

static uint32 __stdcall _wk_thread_entry(void* arg);

void wk_init_threads(void)
{
    main_thread_handle = GetCurrentThread();
    main_thread_id = GetCurrentThreadId();
    tls_is_main_thread = true;
#ifdef WK_DEBUG
    tls_thread_name = "Main";
#endif

    SetThreadPriority(main_thread_handle, WK_THREAD_PRIORITY_HIGH);
}

void wk_quit_threads(void)
{
}

uint32 wk_get_core_count(void)
{
    return GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
}

wk_mutex* wk_create_mutex(void)
{
    wk_mutex* mutex = calloc(1, sizeof(wk_mutex));
    if (!mutex) {
        wk_log_error("Failed to allocate wk_mutex");
        return NULL;
    }

    int init_flags = 0;
#ifndef WK_DEBUG
    init_flags = CRITICAL_SECTION_NO_DEBUG_INFO;
#endif
    if (!InitializeCriticalSectionEx(&mutex->cs, WK_DEFAULT_SPINCOUNT, init_flags)) {
        wk_fatal("InitializeCriticalSectionEx failed with error: %d", GetLastError());
    }
    return mutex;
}

void wk_destroy_mutex(wk_mutex* mutex)
{
    DeleteCriticalSection(&mutex->cs);
    free(mutex);
}

void wk_lock_mutex(wk_mutex* mutex)
{
    EnterCriticalSection(&mutex->cs);
}

void wk_unlock_mutex(wk_mutex* mutex)
{
    LeaveCriticalSection(&mutex->cs);
}

void wk_init_thread(const char* name, wk_thread_func func, wk_thread* thread)
{
    if (InterlockedCompareExchange(&thread->state, WK_THREAD_STATE_RUNNING, WK_THREAD_STATE_READY) != WK_THREAD_STATE_READY)
        return;

    thread->id = _beginthreadex(NULL, 0, _wk_thread_entry, thread, 0, NULL);
    const int error = thread->id ? 0 : GetLastError();

    strncpy(thread->name, name, WK_MAX_THREAD_NAME_LENGTH);

    if (error != 0) {
        thread->state = WK_THREAD_STATE_READY;
        wk_log_error("_beginthreadex failed for thread %s with error: %d", thread->name, error);
    }
}

void wk_quit_thread(wk_thread* thread)
{
    if (thread->state == WK_THREAD_STATE_READY)
        return;

    thread->state = WK_THREAD_STATE_TERMINATING;

    WaitForSingleObject((HANDLE)thread->id, INFINITE);
    CloseHandle((HANDLE)thread->id);

    thread->state = WK_THREAD_STATE_READY;
}

static uint32 __stdcall _wk_thread_entry(void* arg)
{
    wk_thread* thread = arg;
    const HANDLE thread_handle = GetCurrentThread();

    wk_init_time_tls();

    // Enable DAZ and FTZ for performance gains
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    // Priority
    if (thread->priority != 0) {
        SetThreadPriority(thread_handle, thread->priority);
    }

#ifndef WK_RELEASE
    // Name
    tls_thread_name = thread->name;

    wchar_t wide_buffer[WK_MAX_THREAD_NAME_LENGTH];
    const size_t wide_name_length = mbstowcs(NULL, thread->name, 0) + 1;
    wchar_t* wide_name = wide_name_length <= wk_countof(wide_buffer) ? wide_buffer : (wchar_t*)_alloca(wide_name_length * sizeof(wchar_t));
    mbstowcs(wide_name, thread->name, wide_name_length);
    if (FAILED(SetThreadDescription(thread_handle, wide_name))) {
        wk_log_error("SetThreadDescription failed with error: %d", GetLastError());
    }

    // TODO: Also set debug log thread name
#endif

    // Execute
    thread->func(NULL);

    // Terminate
#ifndef WK_RELEASE
    tls_thread_name = NULL;
    // TODO: Also set debug log thread name (NULL)
#endif
    wk_lock_mutex(&thread_mutex);
    InterlockedCompareExchange(&thread->state, WK_THREAD_STATE_IDLE, WK_THREAD_STATE_RUNNING);
    wk_unlock_mutex(&thread_mutex);

    return 0;
}