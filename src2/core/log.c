#include <core/log.h>
#include <core/memory.h>

#include <stdarg.h>
#include <stdio.h>

#define LOG_BUFFER_SIZE 1024

typedef struct log_buffer_t {
    char buffer[LOG_BUFFER_SIZE];
    u32 start;
    u32 end;
} log_buffer_t;

static log_buffer_t logBuffer;
static LogLevel currentLogLevel = LOG_LEVEL;

// Allocator override function pointers (defined in log.h)
void (*log_allocator_free)(void* ptr) = NULL;

bool log_init(const char* file_path, u32 max_mb, u32 flush_ms) {
    // Initialize the log buffer
    logBuffer.start = 0;
    logBuffer.end = 0;
    // Here we could initialize file writing mechanisms, but it’s omitted for simplicity
    return true;
}

void log_shutdown(void) {
    // Flush remaining logs and clean up resources
    log_flush();
    // Omitted actual file operations for simplicity
}

void log_flush(void) {
    // Example flushing
    // Normally, we'd write from logBuffer to a file
    logBuffer.start = 0;
    logBuffer.end = 0;
}

void log_set_level(LogLevel level) {
    currentLogLevel = level;
}

LogLevel log_get_level(void) {
    return currentLogLevel;
}

void log_write(LogLevel level, const char* file, u32 line, const char* func, const char* fmt, ...) {
    if (level < currentLogLevel) {
        return;
    }

    // Handle variadic arguments for formatted string
    va_list args;
    va_start(args, fmt);
    int length = vsnprintf(logBuffer.buffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);

    if (length > 0 && length < LOG_BUFFER_SIZE) {
        logBuffer.end += length;
        // Example logic to manage buffer wrap-around
        if (logBuffer.end >= LOG_BUFFER_SIZE) {
            log_flush();
        }
    }
}

