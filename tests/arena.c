#include <stdio.h>
#define ARENA_REGION_DEFAULT_SIZE 10
#define ARENA_IMPL
#include "../src/arena.h"

struct Foobar {
    size_t foo;
    size_t bar;
};

int main(void) {
    Arena a = {0};

    arena_debug(&a);

    size_t sz = 100;

    char *p = arena_alloc(&a, sizeof(char) * sz);
    arena_debug(&a);

    if (a.end == NULL) {
        fprintf(stderr, "[ERROR]: unable to allocate");
        exit(1);
    }

    for (size_t i = 0; i < sz - 1; ++i) {
        switch (i % 3) {
            case 0: p[i] = 'w'; break;
            case 1: p[i] = 'e'; break;
            case 2: p[i] = 'e'; break;
            default: assert(0 && "unreachable");
        }
    }
    p[sz] = '\0';

    char *pp = arena_alloc(&a, sizeof(char) * sz);

    // assert(a.start != a.end);

    arena_debug(&a);

    for (size_t i = 0; i < sz - 1; ++i) {
        switch (i % 3) {
            case 0: pp[i] = 'w'; break;
            case 1: pp[i] = 'o'; break;
            case 2: pp[i] = 'e'; break;
            default: assert(0 && "unreachable");
        }
    }
    pp[sz] = '\0';

    struct Foobar *f = arena_new(&a, struct Foobar);
    arena_debug(&a);

    f->foo = 10,
    f->bar = 20,
    printf("Foobar: %ld\n", f->foo + f->bar);
    return 0;
}
