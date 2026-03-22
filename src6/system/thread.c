#include "thread.h"
#include "windows.h"
#include <assert.h>

enum
{
    MAX_FIBERS = 128,
    MAX_THREADS = 128,
    MAX_MUTEXES = 256,
    MAX_CONDITIONS = 256,

    DEFAULT_SPIN_COUNT = 1500
};

wb_thread os_create_thread(wb_thread_entry function, void* data)
{
    //HANDLE handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)function, data, 0, 0);
    //assert(handle);
    return (wb_thread) {0};
}

void os_destroy_thread(wb_thread thread)
{
    //WaitForSingleObject(thread->handle, INFINITE);
    //CloseHandle(thread->handle);
}

void os_wait_thread(wb_thread thread)
{
    //WaitForSingleObject(thread->handle, INFINITE);
}

wb_mutex os_create_mutex()
{
    //CRITICAL_SECTION handle;
    //if (!InitializeCriticalSectionAndSpinCount(&handle, DEFAULT_SPIN_COUNT))
    //    assert(0);

    return (wb_mutex) { 0 };
}

void os_destroy_mutex(wb_mutex mutex)
{
    //DeleteCriticalSection(&handle);
}

void os_lock_mutex(wb_mutex mutex)
{
    //EnterCriticalSection(&handle);
}

void os_unlock_mutex(wb_mutex mutex)
{
    //LeaveCriticalSection(&handle);
}

wb_condition os_create_condition(void)
{
    //CONDITION_VARIABLE handle;
    //InitializeConditionVariable(&handle);
    return (wb_condition) {0};
}

void os_wait_condition(wb_condition condition, wb_mutex mutex)
{
    //SleepConditionVariableCS(&condition_handle, &mutex_handle, UINT32_MAX);
}

void os_condition_wake_single(wb_condition condition)
{
    //WakeConditionVariable(&handle);
}

void os_condition_wake_all(wb_condition condition)
{
    //WakeAllConditionVariable(&handle);
}

u32 os_cpu_count(void)
{
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    return system_info.dwNumberOfProcessors;
}

void os_sleep(u32 ms)
{
    Sleep(ms);
}