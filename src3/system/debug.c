#include "../debug.h"
#include "../types.h"
#include "../threads.h"
#include "../file_system.h"
#include "windows_io.h"
#include "windows_misc.h"
#include "windows_file.h"
#include "windows_window.h"
#include "windows_dbghelp.h"
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

#define WK_LOG_BUFFER_SIZE 1024

#define WK_ANSI_RESET  "\x1b[0m"
#define WK_ANSI_RED    "\x1b[31m"
#define WK_ANSI_YELLOW "\x1b[33m"
#define WK_ANSI_GREEN  "\x1b[32m"
#define WK_ANSI_CYAN   "\x1b[36m"
#define WK_ANSI_WHITE  "\x1b[97m"

static bool initialized = false;

#if WK_DEBUG
static wk_log_level log_level = WK_LOG_LEVEL_DEBUG;
#else
static wk_log_level log_level = WK_LOG_LEVEL_ERROR;
#endif
static HANDLE std_output_handle = INVALID_HANDLE_VALUE;
static FILE* log_file;
static char log_file_path[WK_MAX_PATH];

static wk_mutex* log_mutex;

static const char* log_level_names[] = {"DEBUG", "INFO", "ERROR", "FATAL"};
static const char* log_level_colors[] = {WK_ANSI_CYAN, WK_ANSI_WHITE, WK_ANSI_RED, WK_ANSI_RED};

// TODO: Try a MPSC queue instead of the thread local buffers
// TODO: Double buffer input to detect repeated messages (avoid spam)
static __declspec(thread) char log_output[WK_LOG_BUFFER_SIZE];
static __declspec(thread) char log_input[WK_LOG_BUFFER_SIZE];

static FILE* open_log_file(void);

static void __cdecl abort_handler(int code) { wk_fatal("SIGABRT"); }

void wk_init_debug(void)
{
    log_mutex = wk_create_mutex();

    signal(SIGABRT, &abort_handler);

    bool console_valid = AttachConsole(ATTACH_PARENT_PROCESS);
    if (!console_valid) {
        console_valid = AllocConsole();
    }
    if (console_valid) {
        FILE* stream = NULL;
        freopen_s(&stream, "CONOUT$", "w", stdout);
        freopen_s(&stream, "CONOUT$", "w", stderr);
        SetConsoleOutputCP(CP_UTF8);
        std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    log_file = open_log_file();

    initialized = true;
}

void wk_quit_debug(void)
{
    initialized = false;

    fflush(stdout);
    fflush(stderr);
    fflush(log_file);
    if (std_output_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(std_output_handle);
    }
    wk_destroy_mutex(log_mutex);
}

void wk_log(wk_log_level level, const char* file_name, int line, const char* message, ...)
{
    if (!initialized) {
        return;
    }

    if ((int)level < 0 || level >= WK_LOG_LEVEL_COUNT) {
        return;
    }

    if (level < log_level) {
        return;
    }

    char* ptr = log_output;

    va_list args;
    va_start(args, message);
    int length = vsnprintf(log_output, sizeof(log_output) - 2, message, args);
    va_end(args);

    ptr += (length < 0) ? sizeof(log_output) - 2 : length;

    while (ptr > log_output && isspace(ptr[-1])) {
        *--ptr = '\0';
    }
    *ptr = '\0';

    snprintf(log_input, sizeof(log_input), "%s(%d): %s\n", file_name, line, log_output);

    // TODO: Add timestamp to input
    snprintf(log_output, sizeof(log_output), "%s", log_input);

    //wk_lock_mutex(log_mutex);
    if (IsDebuggerPresent) {
        OutputDebugStringA(log_output);
    }
    if (std_output_handle != INVALID_HANDLE_VALUE) {
        if (level == WK_LOG_LEVEL_ERROR)
            SetConsoleTextAttribute(std_output_handle, FOREGROUND_RED);

        DWORD written = 0;
        WriteConsoleA(std_output_handle, log_output, (DWORD)strlen(log_output), &written, NULL);

        if (level == WK_LOG_LEVEL_ERROR)
            SetConsoleTextAttribute(std_output_handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
    if (log_file != NULL) {
        fprintf(log_file, "%s%s", "[TIMESTAMP] ", log_output);
    }
    //wk_unlock_mutex(log_mutex);
}

void wk_fatal(const char* message, ...)
{
    if (!initialized) {
        return;
    }
}

void wk_fatal2(const char* message, ...)
{
#ifdef WK_DEBUG
    OutputDebugStringA(message);
#endif

    if (_isatty(_fileno(stdout))) {
#ifdef WK_DEBUG
        if (IsDebuggerPresent()) {
            OutputDebugStringA("Stack trace:\n");
            fprintf(stderr, "%s\n", "Stack trace:");
            DebugBreak();
            exit(1);
        } else
#endif
        {
            MessageBoxA(NULL, message, "System Error", MB_OK | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST | MB_ICONERROR);
            abort();
        }
    } else {
#ifdef WK_DEBUG
        OutputDebugStringA("Stack trace:\n");
        fprintf(stderr, "%s\n", "Stack trace:");
        if (IsDebuggerPresent()) {
            DebugBreak();
        }
#else
        exit(1);
#endif
    }
}

static FILE* open_log_file(void)
{
    char buffer[MAX_PATH];

    const char* base_dir = wk_get_base_path();
    snprintf(buffer, MAX_PATH, "%s\\..\\logs", base_dir);
    if (!PathIsDirectoryA(buffer)) {
        if (CreateDirectoryA(buffer, 0) == 0) {
            wk_log_error("CreateDirectoryA failed for path %s with error %d", buffer, GetLastError());
            return NULL;
        }
    }
    time_t t = time(NULL);
    struct tm tm = {0};
    localtime_s(&tm, &t);
    snprintf(buffer, MAX_PATH, "%s\\..\\logs\\log%d-%02d-%02d.txt", base_dir, 1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday);

    return fopen(buffer, "a+");
}

void wk_output_debug(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    int length = vsnprintf(log_output, sizeof(log_output) - 2, message, args);
    va_end(args);

    if (length < 0)
        return;

    OutputDebugStringA(log_output);
}
