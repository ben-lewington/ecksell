//  contains:
//      - small associative array
//      - dynamic arrays
//      - logging
//
#ifndef EVERY_H
#define EVERY_H

#define Array(TypeName, Item) typedef \
struct {                              \
    Item *items;                      \
    size_t count;                     \
    size_t cap;                       \
} TypeName

#define ARRAY_DEFAULT_CAPACITY 256

#define array_push(arr, item) do {                                        \
    if ((arr)->count >= (arr)->cap) {                                               \
        (arr)->cap = (arr)->cap == 0 ? ARRAY_DEFAULT_CAPACITY : (arr)->cap * 2;     \
        (arr)->items = realloc((arr)->items, (arr)->cap * sizeof(*((arr)->items))); \
    }                                                                               \
    (arr)->items[(arr)->count++] = (item);                                          \
} while(0)

#define array_extend(arr, its, its_count) do {                               \
    if ((arr)->count + its_count >= (arr)->cap) {                                    \
        if ((arr)->cap == 0) {                                                       \
            (arr)->cap = ARRAY_DEFAULT_CAPACITY;                                     \
        }                                                                            \
        while ((arr)->count + its_count > (arr)->cap) {                              \
            (arr)->cap *= 2;                                                         \
        }                                                                            \
        (arr)->items = realloc((arr)->items, (arr)->cap * sizeof(*((arr)->items)));  \
    }                                                                                \
    memcpy((arr)->items + (arr)->count, (its), its_count * sizeof(*((arr)->items))); \
    (arr)->count += its_count;                                                       \
} while(0)

#define array_free(arr) free((arr)->items)

#endif /* ifndef EVERY_H */
