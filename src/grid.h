#ifndef GRID_H
#define GRID_H
#include "../lib/raylib-5.0_linux_amd64/include/raylib.h"
#include "logging.h"

#define GRID_MOVE_UP    (1 << 0)
#define GRID_MOVE_DOWN  (1 << 1)
#define GRID_MOVE_LEFT  (1 << 2)
#define GRID_MOVE_RIGHT (1 << 3)

typedef struct { int inner; } GridMove;

#include <sys/types.h>

#define GRID_CAST(g, AsTy) (AsTy) { .x = (g).x, .y = (g).y }
#define GRID_PAIRS(g)      (g).x, (g).y
#define GRID_EQU(gl, gr)   (gl).x == (gr).x && (gl.y) == (gr.y)          // l == r
#define GRID_ADD(gl, gr)   { .x = (gl).x + (gr).x, .y = (gl).y + (gr).y } // pointwise addition
#define GRID_SUB(gl, gr)   { .x = (gl).x - (gr).x, .y = (gl).y - (gr).y } // pointwise subtraction
#define GRID_MUL(gl, gr)   { .x = (gl).x * (gr).x, .y = (gl).y * (gr).y } // pointwise multiplication
#define GRID_DIV(gl, gr)   { .x = (gl).x / (gr).x, .y = (gl).y / (gr).y } // pointwise division
#define GRID_SCALE(k,  g)  { .x = (k)    * (g).x,  .y = (k)    * (g).y  } // kv (k is a scalar)
#define GRID_SHEAR(g, n)   { .x = (g).x + (n),     .y = (g).y  + (n)    } // v + (n, n)
#define GRID_FOREACH(g) \
    for (xi = 0; xi <= g.x; ++xi) \
    for (yi = 0; yi <= g.y; ++yi)

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
    Dimensions grid_items;
    Velocity   curs_speed;
} GridCfg;

typedef struct {
    Coord top_left;
    Coord bottom_inset;
} SubRect;

typedef struct  {
    Index            vroot;
    Cursor           vpointer;
    SubRect          window;
    int              grid_pad;
    Dimensions grid_items;
} GridRender;

void grid_render(const GridRender *gr, const Dimensions screen, bool modal);
Dimensions grid_subRectDims(SubRect sr, Dimensions screen);
Index grid_currentIndex(const GridRender *gr);

GridMove grid_captureFrameKeys(void);
#endif // !GRID_H
#define GRID_IMPL
#ifdef GRID_IMPL
Dimensions grid_subRectDims(SubRect sr, Dimensions screen) {
    Dimensions d = GRID_ADD(sr.top_left, sr.bottom_inset);
    return (Dimensions) GRID_SUB(screen, d);
}

Index grid_currentIndex(const GridRender *gr) {
    Offset lattice = GRID_SCALE(1.5, gr->grid_items);
    Cursor lp = GRID_ADD(gr->vpointer, lattice);

    return (Index) {
        .x = (int)(lp.x / gr->grid_items.x) - 1,
        .y = (int)(lp.y / gr->grid_items.y) - 1,
    };
}

Index grid_getRenderSpan(const GridRender *gr, const Dimensions screen) {
    Dimensions grid = grid_subRectDims(gr->window, screen);
    return (Index) {
        .x = (int)(grid.x / gr->grid_items.x) - 1,
        .y = (int)(grid.y / gr->grid_items.y) - 1,
    };
}

void grid_render(const GridRender *gr, const Dimensions screen, bool modal) {
    Offset     lattice = GRID_SCALE(1.5, gr->grid_items);
    Index      max = grid_getRenderSpan(gr, screen);
    Dimensions grid = grid_subRectDims(gr->window, screen);
    DrawText(TextFormat("max: %d, %d", GRID_PAIRS(max)), 0, 30, 10, BLACK);
    int xi, yi;
    GRID_FOREACH(max) {
        Index it = {
            .x = xi,
            .y = yi,
        };

        Coord gi = GRID_MUL(it, gr->grid_items);
        Coord it_pos = GRID_ADD(gr->window.top_left, gi);

        // rendering the outline
        if (xi == 0 || yi == 0) {
            // rendering the current cell indicator
            if (xi == 0 && yi == 0) {
                Cursor gpad = GRID_SHEAR(gr->window.top_left, gr->grid_pad);
                DrawText(
                        TextFormat("%d, %d", GRID_PAIRS(gr->vroot)),
                        GRID_PAIRS(gpad), 10, BLACK);

            }
            if (xi > 0)
                DrawText(TextFormat("%d", gr->vroot.x + xi - 1),
                        it_pos.x + gr->grid_pad, gr->window.top_left.x + gr->grid_pad, 10, BLACK);
            if (yi > 0)
                DrawText(TextFormat("%d", gr->vroot.y + yi - 1),
                        gr->window.top_left.y + gr->grid_pad, it_pos.y + gr->grid_pad, 10, BLACK);
        }

        if (xi == 0)
            DrawLineEx((Vector2) { .x = gr->window.top_left.x,          .y = it_pos.y },
                       (Vector2) { .x = gr->window.top_left.x + grid.x, .y = it_pos.y },
                       1, BLACK);

        DrawLineEx((Vector2) { .x = it_pos.x, .y = gr->window.top_left.y },
                   (Vector2) { .x = it_pos.x, .y = gr->window.top_left.y + grid.y },
                   1, BLACK);

        Cursor lp = GRID_ADD(gr->vpointer, lattice);
        DrawText(TextFormat("vpointer: (%f, %f), lattice: (%f, %f)",
                    GRID_PAIRS(gr->vpointer), GRID_PAIRS(lp)), 0, 0, 10, BLACK);
        Index lpi = {
            .x = (int)(lp.x / gr->grid_items.x) - 1,
            .y = (int)(lp.y / gr->grid_items.y) - 1,
        };
        DrawText(TextFormat("calc index: %d, %d", GRID_PAIRS(lpi)), 0, 20, 10, BLACK);
        Index gid = GRID_SHEAR(it, -1);
        if (GRID_EQU(lpi, gid)) {
            if (!modal) DrawRectangleLinesEx((Rectangle) {
                .x = it_pos.x,
                .y = it_pos.y,
                .width = gr->grid_items.x,
                .height = gr->grid_items.y,
            }, gr->grid_pad / 2., GREEN);
        }
    }
    Index mm = GRID_SHEAR(max, 1);
    Coord ll = GRID_MUL(gr->grid_items, mm);
    Cursor f = GRID_ADD(ll, gr->window.top_left);
    DrawLineEx((Vector2) {.x = gr->window.top_left.x, .y = f.y},
               (Vector2) {.x = gr->window.top_left.x + grid.x, .y = f.y},
               1, BLACK);
    DrawLineEx((Vector2) {.x = f.x, .y = gr->window.top_left.y},
               (Vector2) {.x = f.x, .y = gr->window.top_left.y + grid.y},
               1, BLACK);

    // DrawRectangle(1, 0, screen.x, gr->window.top_left.y, GRAY);
    // DrawRectangle(0, 0, gr->window.top_left.x, screen.y, GRAY);
    // DrawRectangle(0, screen.y - gr->window.bottom_inset.y, screen.x, screen.y, GRAY);
    // DrawRectangle(screen.x - gr->window.bottom_inset.x, 0, screen.x, screen.y, GRAY);

}

GridMove grid_captureFrameKeys(void) {
    GridMove ret = {0};
    if (IsKeyDown(KEY_L)) ret.inner |= GRID_MOVE_RIGHT;
    if (IsKeyDown(KEY_H)) ret.inner |= GRID_MOVE_LEFT;
    if (IsKeyDown(KEY_J)) ret.inner |= GRID_MOVE_DOWN;
    if (IsKeyDown(KEY_K)) ret.inner |= GRID_MOVE_UP;
    return ret;
}

#endif // GRID_IMPL
