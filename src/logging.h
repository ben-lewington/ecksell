#ifndef LOG_H
#define LOG_H

#ifndef __log_impl
#include <stdio.h>
#define __log_impl(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
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

#define __ldebug(fmt, ...) do { __log_impl("[DEBUG]: %s:%d: " fmt "\n", __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); } while (0)
#define __linfo(fmt, ...)  do { __log_impl("[INFO] : %s:%d: " fmt "\n", __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); } while (0)
#define __lwarn(fmt, ...)  do { __log_impl("[WARN] : %s:%d: " fmt "\n", __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); } while (0)
#define __lerror(fmt, ...) do { __log_impl("[ERROR]: %s:%d: " fmt "\n", __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); } while (0)

#ifndef LOG_LEVEL
    #define log_debug(fmt, ...) __ldebug(fmt, __VA_ARGS__)
    #define log_info(fmt, ...)  __linfo(fmt, __VA_ARGS__)
    #define log_warn(fmt, ...)  __lwarn(fmt, __VA_ARGS__)
    #define log_error(fmt, ...) __lerror(fmt, __VA_ARGS__)
#else
    #if ((LOG_LEVEL) <= LOG_DEBUG)
        #define every_debug(fmt, ...) __ldebug(fmt, __VA_ARGS__)
    #else
        #define every_debug(fmt, ...)
    #endif
    #if ((EVERY_LOG_LEVEL) <= LOG_INFO)
        #define every_info(fmt, ...)  __linfo(fmt, __VA_ARGS__)
    #else
        #define every_info(fmt, ...)
    #endif
    #if ((EVERY_LOG_LEVEL) <= LOG_WARN) // LOG_WARN
        #define every_warn(fmt, ...)  __lwarn(fmt, __VA_ARGS__)
    #else
        #define every_warn(fmt, ...)
    #endif
#endif
#endif // LOG_H

#ifdef LOG_RT_IMPL
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
#endif // LOG_RT_IMPL
