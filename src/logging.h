#ifndef LOG_H
#define LOG_H

#include <assert.h>
#include <stdio.h>

#ifndef __log_impl
#define __log_impl(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)
#endif // __log_impl

#define LOG_DEBUG 0
#define LOG_INFO  1
#define LOG_WARN  2
#define LOG_ERROR 3

typedef enum {
    debug = LOG_DEBUG,
    info  = LOG_INFO,
    warn  = LOG_WARN,
    error = LOG_ERROR,
} LogLevel;

/// log a value at any log level at runtime
void log_fmt(LogLevel lvl, char *fmt, ...);

#define __ldebug(fmt, ...) __log_impl("[DEBUG]: %s:%d: " fmt "\n", __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define __linfo(fmt, ...)  __log_impl("[INFO] : %s:%d: " fmt "\n", __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define __lwarn(fmt, ...)  __log_impl("[WARN] : %s:%d: " fmt "\n", __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define __lerror(fmt, ...) __log_impl("[ERROR]: %s:%d: " fmt "\n", __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)

#ifndef LOG_LEVEL
    #define log_debug(fmt, ...) __ldebug(fmt, __VA_ARGS__)
    #define log_info(fmt, ...)  __linfo(fmt, __VA_ARGS__)
    #define log_warn(fmt, ...)  __lwarn(fmt, __VA_ARGS__)
    #define log_error(fmt, ...) __lerror(fmt, __VA_ARGS__)
#else
    #if ((LOG_LEVEL) <= LOG_DEBUG)
        #define log_debug(fmt, ...) __ldebug(fmt, __VA_ARGS__)
    #else
        #define every_debug(fmt, ...)
    #endif
    #if ((EVERY_LOG_LEVEL) <= LOG_INFO)
        #define log_info(fmt, ...)  __linfo(fmt, __VA_ARGS__)
    #else
        #define every_info(fmt, ...)
    #endif
    #if ((EVERY_LOG_LEVEL) <= LOG_WARN) // LOG_WARN
        #define log_warn(fmt, ...)  __lwarn(fmt, __VA_ARGS__)
    #else
        #define every_warn(fmt, ...)
    #endif
    #define log_error(fmt, ...)     __lerror(fmt, __VA_ARGS__)
#endif
#endif // LOG_H

#ifdef LOGGING_IMPL
/// log a value at any log level at runtime
void log_fmt(LogLevel lvl, char *fmt, ...) {
    switch (lvl) {
        case LOG_DEBUG:
            fprintf(stderr, "[DEBUG]"); break;
        case LOG_INFO:
            fprintf(stderr, "[INFO] "); break;
        case LOG_WARN:
            fprintf(stderr, "[WARN] "); break;
        case LOG_ERROR:
            fprintf(stderr, "[ERROR]"); break;
        default:
            assert(0 && "unreachable");
    }
    fprintf(stderr, "::: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}
#endif // LOGGING_IMPL
