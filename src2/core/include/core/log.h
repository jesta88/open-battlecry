#pragma once

#include <core/types.h>

// Log level enumeration (ordered by severity)
typedef enum LogLevel {
    LOG_TRACE = 0,
    LOG_DEBUG = 1,
    LOG_INFO  = 2,
    LOG_WARN  = 3,
    LOG_ERROR = 4,
    LOG_FATAL = 5,
    LOG_LEVEL_COUNT
} LogLevel;

// Default compile-time log level (can be overridden via -DLOG_LEVEL=...)
#ifndef LOG_LEVEL
    #ifdef NDEBUG
        #define LOG_LEVEL LOG_INFO
    #else
        #define LOG_LEVEL LOG_TRACE
    #endif
#endif

//-------------------------------------------------------------------------------------------------
// Core logging functions

// Initialize logging system
// Returns false on failure (invalid path, permissions, etc.)
bool log_init(const char* file_path, u32 max_mb, u32 flush_ms);

// Shutdown logging system and flush all pending messages
void log_shutdown(void);

// Manually flush log buffer to disk
void log_flush(void);

// Set minimum log level (messages below this level will be ignored)
void log_set_level(LogLevel level);

// Get current log level
LogLevel log_get_level(void);

//-------------------------------------------------------------------------------------------------
// Core logging macro

// Main logging macro that captures file, line, and function information
#define LOG(level, fmt, ...) \
    do { \
        if ((level) >= LOG_LEVEL) { \
            log_write((level), __FILE__, __LINE__, __func__, (fmt), ##__VA_ARGS__); \
        } \
    } while (0)

// Internal logging function (not intended for direct use)
void log_write(LogLevel level, const char* file, u32 line, const char* func, const char* fmt, ...);

//-------------------------------------------------------------------------------------------------
// Convenience macros (compile out when level is disabled)

#if LOG_LEVEL <= LOG_TRACE
    #define LOG_TRACE(fmt, ...) LOG(LOG_TRACE, fmt, ##__VA_ARGS__)
#else
    #define LOG_TRACE(fmt, ...) ((void)0)
#endif

#if LOG_LEVEL <= LOG_DEBUG
    #define LOG_DEBUG(fmt, ...) LOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#else
    #define LOG_DEBUG(fmt, ...) ((void)0)
#endif

#if LOG_LEVEL <= LOG_INFO
    #define LOG_INFO(fmt, ...)  LOG(LOG_INFO, fmt, ##__VA_ARGS__)
#else
    #define LOG_INFO(fmt, ...)  ((void)0)
#endif

#if LOG_LEVEL <= LOG_WARN
    #define LOG_WARN(fmt, ...)  LOG(LOG_WARN, fmt, ##__VA_ARGS__)
#else
    #define LOG_WARN(fmt, ...)  ((void)0)
#endif

#if LOG_LEVEL <= LOG_ERROR
    #define LOG_ERROR(fmt, ...) LOG(LOG_ERROR, fmt, ##__VA_ARGS__)
#else
    #define LOG_ERROR(fmt, ...) ((void)0)
#endif

#if LOG_LEVEL <= LOG_FATAL
    #define LOG_FATAL(fmt, ...) LOG(LOG_FATAL, fmt, ##__VA_ARGS__)
#else
    #define LOG_FATAL(fmt, ...) ((void)0)
#endif

//-------------------------------------------------------------------------------------------------
// Future extensibility

// Allocator override function pointer (for future use)
// Set to NULL to use default allocator
extern void* (*log_allocator_malloc)(u32 size);
extern void (*log_allocator_free)(void* ptr);
