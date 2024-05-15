#ifndef TYPES_H
#define TYPES_H

#include <sys/types.h>

#define GridCoords(Name, Idx) \
    typedef struct { Idx x; Idx y; } Name

GridCoords(IndexStrict, size_t);
GridCoords(Index,       int);
GridCoords(Cursor,      float);
GridCoords(Coord,       float);
GridCoords(Offset,      float);
GridCoords(Dimensions,  float);
GridCoords(Velocity,    float);

typedef struct {
    Coord top_left;
    Coord bottom_inset;
} Window;

#define GRID_FMT_STRF      "%f, %f"
#define GRID_FMT_STRI      "%d, %d"
#define GRID_CAST(g, AsTy) (AsTy) { .x = (g).x, .y = (g).y }
#define GRID_PAIRS(g)     (g).x, (g).y
#define GRID_EQU(gl, gr)  (gl).x == (gr).x && (gl.y) == (gr.y)          // l == r
#define GRID_ADD(gl, gr)  { .x = (gl).x + (gr).x, .y = (gl).y + (gr).y } // pointwise addition
#define GRID_SUB(gl, gr)  { .x = (gl).x - (gr).x, .y = (gl).y - (gr).y } // pointwise subtraction
#define GRID_MUL(gl, gr)  { .x = (gl).x * (gr).x, .y = (gl).y * (gr).y } // pointwise multiplication
#define GRID_DIV(gl, gr)  { .x = (gl).x / (gr).x, .y = (gl).y / (gr).y } // pointwise division
#define GRID_SCALE(k, g)  { .x = (k)    * (g).x,  .y = (k)    * (g).y  } // kv (k is a scalar)
#define GRID_SHEAR(g, n)  { .x = (g).x + (n),     .y = (g).y  + (n)    } // v + (n, n)
#define GRID_IADD(g, gr)  do { (g) = { .x = (g).x + (gr).x, .y = (g).y + (gr).y } } while (0)  // inplace pointwise addition
#define GRID_ISUB(g, gr)  do { (g) = { .x = (g).x - (gr).x, .y = (g).y - (gr).y } } while (0)  // inplace pointwise subtraction
#define GRID_IMUL(g, gr)  do { (g) = { .x = (g).x * (gr).x, .y = (g).y * (gr).y } } while (0)  // inplace pointwise multiplication
#define GRID_IDIV(g, gr)  do { (g) = { .x = (g).x / (gr).x, .y = (g).y / (gr).y } } while (0)  // inplace pointwise division
#define GRID_ISCALE(k, g) do { (g) = { .x = (k)    * (g).x, .y = (k)    * (g).y  } } while (0) // inplace kv (k is a scalar)
#define GRID_ISHEAR(g, n) do { (g) = { .x = (g).x + (n),    .y = (g).y  + (n)    } } while (0) // inplace v + (n, n)
#define GRID_FOREACH(g) \
    for (xi = 0; xi <= g.x; ++xi) \
    for (yi = 0; yi <= g.y; ++yi)

Dimensions window_dimensions(Window sr, Dimensions screen);
#endif // !TYPES_H
// #define TYPES_IMPL
#ifdef TYPES_IMPL
Dimensions window_dimensions(Window sr, Dimensions screen) {
    Dimensions d = GRID_ADD(sr.top_left, sr.bottom_inset);
    return (Dimensions) GRID_SUB(screen, d);
}
#endif // TYPES_IMPL
