//  contains:
//      - small associative array
//      - dynamic arrays
//      - logging
//
#ifndef ARRAY_H
#define ARRAY_H

#include <sys/types.h>

#define Array(TypeName, Item) typedef \
struct {                              \
    Item *inner;                      \
    size_t len;                       \
    size_t cap;                       \
} TypeName

Array(String, char);

// use libc malloc
// #define INTO_ARENA
#ifndef INTO_ARENA

#define ARRAY_DEFAULT_CAPACITY 256

#define array_push(arr, item) do {                                                  \
    if ((arr)->len >= (arr)->cap) {                                                 \
        (arr)->cap = (arr)->cap == 0 ? ARRAY_DEFAULT_CAPACITY : (arr)->cap * 2;     \
        (arr)->inner = realloc((arr)->inner, (arr)->cap * sizeof(*((arr)->inner))); \
    }                                                                               \
    (arr)->inner[(arr)->len++] = (item);                                            \
} while(0)

#define array_extend(arr, its, its_count) do {                                       \
    if ((arr)->len + its_count >= (arr)->cap) {                                      \
        if ((arr)->cap == 0) {                                                       \
            (arr)->cap = ARRAY_DEFAULT_CAPACITY;                                     \
        }                                                                            \
        while ((arr)->len + its_count > (arr)->cap) {                                \
            (arr)->cap *= 2;                                                         \
        }                                                                            \
        (arr)->inner = realloc((arr)->inner, (arr)->cap * sizeof(*((arr)->inner)));  \
    }                                                                                \
    memcpy((arr)->inner + (arr)->len, (its), its_count * sizeof(*((arr)->inner)));   \
    (arr)->len += its_count;                                                         \
} while(0)

#define array_free(arr) free((arr)->inner)

// use backing arena
#else

#define ARRAY_DEFAULT_CAPACITY (1024 * 4)

#define array_push(arena, arr, item) do {                                       \
    if ((arr)->len >= (arr)->cap) {                                             \
        size_t cur_cap = (arr)->cap;                                            \
        (arr)->cap = (arr)->cap == 0 ? ARRAY_DEFAULT_CAPACITY : (arr)->cap * 2; \
        (arr)->inner = arena_realloc((arena), (arr)->inner, cur_cap,            \
                                     (arr)->cap * sizeof(*((arr)->inner)));     \
    }                                                                           \
    (arr)->inner[(arr)->len++] = (item);                                        \
} while(0)

#define array_extend(arena, arr, its, its_count) do {                                              \
    if ((arr)->len + its_count >= (arr)->cap) {                                                    \
        if ((arr)->cap == 0) {                                                                     \
            (arr)->cap = ARRAY_DEFAULT_CAPACITY;                                                   \
        }                                                                                          \
        while ((arr)->len + its_count > (arr)->cap) {                                              \
            (arr)->cap *= 2;                                                                       \
        }                                                                                          \
        (arr)->inner = arena_realloc((arena), (arr)->inner, (arr)->cap * sizeof(*((arr)->inner))); \
    }                                                                                              \
    memcpy((arr)->inner + (arr)->len, (its), its_count * sizeof(*((arr)->inner)));                 \
    (arr)->len += its_count;                                                                       \
} while(0)

#define array_free(arr)

#endif // !INTO_ARENA
#endif // !ARRAY_H
