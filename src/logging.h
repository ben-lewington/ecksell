#ifndef LOG_H
#define LOG_H

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#ifndef LOG_FMT_WRITE
#define LOG_FMT_WRITE(fmt, ...) fprintf(stderr, fmt "\n"  __VA_OPT__(,) __VA_ARGS__)
#endif // LOG_FMT_WRITE

#define LOG_DEBUG 0
#define LOG_INFO  1
#define LOG_WARN  2
#define LOG_ERROR 3

/// log a value at any log level at runtime
void    log_fmt(int lvl, char *fmt, ...);

#define log_error(fmt, ...) LOG_FMT_WRITE("[ERROR] %s:%d: " fmt, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define log_debug(fmt, ...) LOG_FMT_WRITE("[DEBUG] %s:%d: " fmt, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define log_info(fmt, ...)  LOG_FMT_WRITE("[INFO]: "        fmt                     __VA_OPT__(,) __VA_ARGS__)
#define log_warn(fmt, ...)  LOG_FMT_WRITE("[WARN]: "        fmt                     __VA_OPT__(,) __VA_ARGS__)

#ifdef LOG_LEVEL
#if ((LOG_LEVEL) > LOG_DEBUG)
#undef  log_debug
#define log_debug(fmt, ...)
#elif ((LOG_LEVEL > LOG_INFO))
#undef  log_info
#define log_info(fmt, ...)
#elif ((LOG_LEVEL) > LOG_WARN)
#undef  log_warn
#define log_warn(fmt, ...)
#endif
#endif // !LOG_LEVEL
#endif // LOG_H

// #define LOGGING_IMPL
#ifdef LOGGING_IMPL
/// log a value at any log level at runtime
void log_fmt(int lvl, char *fmt, ...) {
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
