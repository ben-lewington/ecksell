#ifndef ARENA_H
#define ARENA_H

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef ARENA_MEM_DEFAULT_SIZE
#define ARENA_MEM_DEFAULT_SIZE (8 * 1024);
#endif

typedef struct Mem Mem;
struct Mem {
    Mem *next;
    size_t len;
    size_t cap;
    uintptr_t raw[];
};

#define ARENA_MEM_FSTR "Memory { capacity: %ld, len: %ld, next?: %s }"
#define ARENA_MEM_FMT(a) (a)->cap, (a)->len, ((a)->next != NULL) ? "true" : "false"
#define arena_new(a, Type) arena_alloc((a), sizeof(Type))

typedef struct Arena Arena;
struct Arena { Mem *start, *end; };

Mem  *__arena_mem_new(size_t cap);
void  __arena_mem_free(Mem *r);

void *arena_alloc(Arena *a, size_t sz_bytes);
void *arena_realloc(Arena *a, void *cur, size_t cur_sz_bytes, size_t new_sz_bytes);

void  arena_reset(Arena *a);
void  arena_free(Arena *a);

int   arena_snapshot(const Arena *a);
int   arena_rollback(Arena *a, int snapshot);

void  arena_debug(const Arena *a);

#endif // !ARENA_H

// #define ARENA_IMPL
#ifdef ARENA_IMPL

Mem *__arena_mem_new(size_t cap) {
    size_t sz = sizeof(Mem) + sizeof(int8_t) * cap;

    Mem *r = (Mem *)malloc(sz);
    r->len = 0;
    r->cap = cap;
    memset(&r->raw, 0, cap);
    assert(r && "OOM");

    return r;
}

void __arena_mem_free(Mem *r) { free(r); }

void *arena_alloc(Arena *a, size_t sz_bytes) {
    // if we need to allocate a new region, the default cap best be big enough.
    size_t cap = ARENA_MEM_DEFAULT_SIZE;
    if (sz_bytes * sizeof(int8_t)  > cap) cap = sz_bytes;

    void *ret = NULL;
    if (a->end == NULL) {
        // if there's no regions yet, allocate one
        a->end = __arena_mem_new(cap);
        a->start = a->end;
    }

    while (a->end->len + sz_bytes > a->end->cap && a->end != NULL) {
        // here, we want to move to either the end of the arena, or a region with space for our memory
        if (a->end->next == NULL) {
            a->end->next = __arena_mem_new(cap);
            a->end = a->end->next;
            break;
        }
        a->end = a->end->next;
    }

    // here we know a->end is not NULL and (a->end->cap - a->end->len) > sz_bytes, so we take a pointer
    // to the top of the arena and bump the len.
    ret = &a->end->raw[a->end->len];
    a->end->len += sz_bytes;
    return ret;
}

void *arena_realloc(Arena *a, void *cur, size_t cur_sz_bytes, size_t new_sz_bytes) {
    if (new_sz_bytes <= cur_sz_bytes) return cur;
    void *newp = arena_alloc(a, new_sz_bytes);
    if (cur != NULL) memcpy(newp, cur, cur_sz_bytes);
    return newp;
}

void arena_reset(Arena *a) {
    for (Mem *m = a->start; m != NULL; m = m->next) {
        m->len = 0; a->end = a->start;
    }
}

void arena_free(Arena *a) {
    Mem *r = a->start;
    while (r) {
        Mem *rr = r;
        r = r->next;
        __arena_mem_free(rr);
    }
    a->start = NULL;
    a->end = NULL;
}

int arena_snapshot(const Arena *a) {
    if (a->start == NULL) return -1;
    int ret = 0;
    const Mem *m = a->start;
    ret += m->len;
    while (m->next != NULL) {
        m = m->next;
        ret += m->len;
    }
    return ret;
}

int arena_rollback(Arena *a, int snapshot) {
    if (snapshot < 0 || a->start == NULL) return -1;

    Mem *m = a->start;
    while (m->next != NULL && snapshot >= 0) {
        int ml = m->len;
        if (snapshot <= ml) break;

        snapshot -= ml;
        m = m->next;
    }

    if (snapshot <= 0) return 0;
    int cleared = 0;

    m->len = snapshot;
    cleared = m->len - snapshot;
    while (m->next != NULL) {
        cleared += m->len;
        m->len = 0;
        m = m->next;
    }
    return cleared;
}

void arena_debug(const Arena *a) {
    fprintf(stderr, "Arena: {");
    if (a->end == NULL) {
        if (a->start != NULL) {
            fprintf(stderr, " Corrupted (begin != end) }\n");
        } else {
            fprintf(stderr, " Uninitialised }\n");
        }
    } else {
        Mem *m = a->start;
        if (m == NULL) {
            fprintf(stderr, " Corrupted (begin != end) ");
        } else while (m != NULL) {
            fprintf(stderr, "\n    ->\n        " ARENA_MEM_FSTR "\n", ARENA_MEM_FMT(m));
            m = m->next;
        }
        fprintf(stderr, "}\n");
    }
}

#endif // ARENA_IMPL
