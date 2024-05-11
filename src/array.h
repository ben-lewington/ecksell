//  contains:
//      - small associative array
//      - dynamic arrays
//      - logging
//
#ifndef EVERY_H
#define EVERY_H

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define every_dyn_arr(ident, T) typedef \
struct {                                \
    T *items;                           \
    size_t count;                       \
    size_t cap;                         \
} ident

#define EVERY_DYN_ARR_INIT_CAP 256

#define every_dyn_arr_append(arr, item) do {                                        \
    if ((arr)->count >= (arr)->cap) {                                               \
        (arr)->cap = (arr)->cap == 0 ? EVERY_DYN_ARR_INIT_CAP : (arr)->cap * 2;     \
        (arr)->items = realloc((arr)->items, (arr)->cap * sizeof(*((arr)->items))); \
    }                                                                               \
    (arr)->items[(arr)->count++] = (item);                                          \
} while(0)

#define every_dyn_arr_extend(arr, its, its_count) do {                               \
    if ((arr)->count + its_count >= (arr)->cap) {                                    \
        if ((arr)->cap == 0) {                                                       \
            (arr)->cap = EVERY_DYN_ARR_INIT_CAP;                                     \
        }                                                                            \
        while ((arr)->count + its_count > (arr)->cap) {                              \
            (arr)->cap *= 2;                                                         \
        }                                                                            \
        (arr)->items = realloc((arr)->items, (arr)->cap * sizeof(*((arr)->items)));  \
    }                                                                                \
    memcpy((arr)->items + (arr)->count, (its), its_count * sizeof(*((arr)->items))); \
    (arr)->count += its_count;                                                       \
} while(0)

#define every_dyn_arr_free(arr) free((arr)->items)

#ifdef EVERY_IMPL

#endif /* ifndef EVERY_IMPL */
#endif /* ifndef EVERY_H */
