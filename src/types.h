#ifndef TYPES_H
#define TYPES_H

#include <sys/types.h>
#include <raylib.h>

#include "array.h"
#include "arena.h"

// TODO(BL): namespace these types everywhere, i.e. Grid ## Name
#define GridCoords(Name, Idx) \
    typedef struct { Idx x; Idx y; } Name

GridCoords(IndexStrict, size_t);
GridCoords(Index,       int);
GridCoords(Cursor,      float);
GridCoords(Coord,       float);
GridCoords(Offset,      float);
GridCoords(Dimensions,  float);
GridCoords(Velocity,    float);

typedef void (*RenderText)(const char*, Vector2, float, float, Color);

typedef enum {
    MODE_NORMAL,
    MODE_INSERT,
    MODE_FIND,
    MODES_COUNT,
} AppMode;

typedef struct AppState AppState;
struct AppState {
    AppMode mode;
    Dimensions screen;
    RenderText write_text;
};

typedef struct {
    int        pad;
    Dimensions grid_item;
    Velocity   curs_speed;
} AppCfg;

typedef struct {
    Coord top_left;
    Coord bottom_inset;
} Window;

typedef enum {
    CELL_TYPE_RAW,
    CELL_TYPE_NUMBER,
    CELL_TYPE_REAL,
    CELL_TYPE_TEXT,
} CellType;

typedef union {
    void       *raw;
    size_t      number;
    float       real;
    const char *text;
} CellValue;

typedef struct {
    CellType  type;
    CellValue as;
} Cell;

typedef struct {
    Index     grid_id;
    CellType  type;
    CellValue as;
} GridCell;

Array(GridCells, GridCell);

typedef struct {
    Arena    *alc;
    GridCells cells;
} CellStorage;

#define GRID_FMT_STRF "%f, %f"
#define GRID_FMT_STRI "%d, %d"
#define GRID_PAIRS(g) (g).x, (g).y
#define GRID_CAST(g, AsTy) (AsTy) { .x = (g).x, .y = (g).y }
#define GRID_EQU(gl, gr)  (gl).x == (gr).x && (gl.y) == (gr.y)                             // l == r
#define GRID_NEW(xi, yi)  { .x = (xi),            .y = (yi)            }                   // constructor
#define GRID_ADD(gl, gr)  { .x = (gl).x + (gr).x, .y = (gl).y + (gr).y }                   // pointwise addition
#define GRID_SUB(gl, gr)  { .x = (gl).x - (gr).x, .y = (gl).y - (gr).y }                   // pointwise subtraction
#define GRID_MUL(gl, gr)  { .x = (gl).x * (gr).x, .y = (gl).y * (gr).y }                   // pointwise multiplication
#define GRID_DIV(gl, gr)  { .x = (gl).x / (gr).x, .y = (gl).y / (gr).y }                   // pointwise division
#define GRID_SCALE(k, g)  { .x = (k)    * (g).x,  .y = (k)    * (g).y  }                   // kv (k is a scalar)
#define GRID_SHEAR(g, n)  { .x = (g).x + (n),     .y = (g).y  + (n)    }                   // v + (n, n)
#define GRID_IADD(g, gr)  do { (g).x = (g).x + (gr).x; (g).y = (g).y + (gr).y; } while (0) // inplace pointwise addition
#define GRID_ISUB(g, gr)  do { (g).x = (g).x - (gr).x; (g).y = (g).y - (gr).y; } while (0) // inplace pointwise subtraction
#define GRID_IMUL(g, gr)  do { (g).x = (g).x * (gr).x; (g).y = (g).y * (gr).y; } while (0) // inplace pointwise multiplication
#define GRID_IDIV(g, gr)  do { (g).x = (g).x / (gr).x; (g).y = (g).y / (gr).y; } while (0) // inplace pointwise division
#define GRID_ISCALE(k, g) do { (g).x = (k)    * (g).x; (g).y = (k)    * (g).y; } while (0) // inplace kv (k is a scalar)
#define GRID_ISHEAR(g, n) do { (g).x = (g).x + (n)   ; (g).y = (g).y  + (n)  ; } while (0) // inplace v + (n, n)
#define GRID_FOREACH(g) \
    for (xi = 0; xi <= g.x; ++xi) \
    for (yi = 0; yi <= g.y; ++yi)

#define RECT_PAD(rect, pad) do { \
    (rect).x = (rect).x + (pad); \
    (rect).y = (rect).y + (pad); \
    (rect).width -= 2 * (pad); \
    (rect).height -= 2 * (pad); \
} while (0) // inplace v + (n, n)

#define TO_RECT(top_left, dims) (Rectangle) { .x = (top_left).x, .y = (top_left).y, .width = (dims).x, .height = (dims).y }

Dimensions window_dimensions(Window sr, Dimensions screen);
#endif // !TYPES_H
// #define TYPES_IMPL
#ifdef TYPES_IMPL
Dimensions window_dimensions(Window sr, Dimensions screen) {
    Dimensions d = GRID_ADD(sr.top_left, sr.bottom_inset);
    return (Dimensions) GRID_SUB(screen, d);
}
#endif // TYPES_IMPL
