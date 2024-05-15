#ifndef GRID_H
#define GRID_H
#include "../lib/raylib-5.0_linux_amd64/include/raylib.h"
#include "logging.h"
#include "types.h"

#define GRID_MOVE_UP    (1 << 0)
#define GRID_MOVE_DOWN  (1 << 1)
#define GRID_MOVE_LEFT  (1 << 2)
#define GRID_MOVE_RIGHT (1 << 3)

typedef struct { int inner; } GridMove;

typedef struct {
    int        grid_pad;
    Dimensions grid_item;
    Velocity   curs_speed;
} AppCfg;

typedef struct  {
    Index  vroot;
    Cursor vpointer;
    Window window;
} GridRender;

void       grid_render(const GridRender *gr, const Dimensions screen, const AppCfg *cfg, bool modal);

Dimensions grid_windowSize(Window sr, Dimensions screen);
Index      grid_currentIndex(const GridRender *gr, const Dimensions grid_item);
GridMove   grid_captureFrameKeys(void);

#endif // !GRID_H
// #define GRID_IMPL
#ifdef GRID_IMPL
Dimensions grid_windowSize(Window sr, Dimensions screen) {
    Dimensions d = GRID_ADD(sr.top_left, sr.bottom_inset);
    return (Dimensions) GRID_SUB(screen, d);
}

Index grid_currentIndex(const GridRender *gr, const Dimensions grid_item) {
    Offset lattice = GRID_SCALE(1.5, grid_item);
    Cursor lp = GRID_ADD(gr->vpointer, lattice);

    return (Index) {
        .x = (int)(lp.x / grid_item.x) - 1,
        .y = (int)(lp.y / grid_item.y) - 1,
    };
}

Index grid_getRenderSpan(const GridRender *gr, const Dimensions screen, const AppCfg *cfg) {
    Dimensions grid = grid_windowSize(gr->window, screen);
    return (Index) {
        .x = (int)(grid.x / cfg->grid_item.x) - 1,
        .y = (int)(grid.y / cfg->grid_item.y) - 1,
    };
}

void grid_render(const GridRender *gr, const Dimensions screen, const AppCfg *cfg, bool modal) {
    Offset     lattice = GRID_SCALE(1.5, cfg->grid_item);
    Index      max = grid_getRenderSpan(gr, screen, cfg);
    Dimensions grid = grid_windowSize(gr->window, screen);
    DrawRectangleLines(GRID_PAIRS(gr->window.top_left), GRID_PAIRS(grid), BLACK);
    DrawText(TextFormat("max: %d, %d", GRID_PAIRS(max)), 0, 30, 10, BLACK);
    int xi, yi;
    GRID_FOREACH(max) {
        Index it = {
            .x = xi,
            .y = yi,
        };

        Coord gi = GRID_MUL(it, cfg->grid_item);
        Coord it_pos = GRID_ADD(gr->window.top_left, gi);

        // rendering the outline
        if (xi == 0 || yi == 0) {
            // rendering the current cell indicator
            if (xi == 0 && yi == 0) {
                Cursor gpad = GRID_SHEAR(gr->window.top_left, cfg->grid_pad);
                DrawText(TextFormat("%d, %d", GRID_PAIRS(gr->vroot)),
                         GRID_PAIRS(gpad), 10, BLACK);

            }
            if (xi > 0)
                DrawText(TextFormat("%d", gr->vroot.x + xi - 1),
                        it_pos.x + cfg->grid_pad, gr->window.top_left.x + cfg->grid_pad, 10, BLACK);
            if (yi > 0)
                DrawText(TextFormat("%d", gr->vroot.y + yi - 1),
                        gr->window.top_left.y + cfg->grid_pad, it_pos.y + cfg->grid_pad, 10, BLACK);
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
            .x = (int)(lp.x / cfg->grid_item.x) - 1,
            .y = (int)(lp.y / cfg->grid_item.y) - 1,
        };
        DrawText(TextFormat("calc index: %d, %d", GRID_PAIRS(lpi)), 0, 20, 10, BLACK);
        Index gid = GRID_SHEAR(it, -1);
        if (GRID_EQU(lpi, gid)) {
            if (!modal) DrawRectangleLinesEx((Rectangle) {
                .x = it_pos.x,
                .y = it_pos.y,
                .width = cfg->grid_item.x,
                .height = cfg->grid_item.y,
            }, cfg->grid_pad / 2., GREEN);
        }
    }

    Index mm = GRID_SHEAR(max, 1);
    Coord ll = GRID_MUL(cfg->grid_item, mm);
    Cursor f = GRID_ADD(ll, gr->window.top_left);

    DrawLineEx((Vector2) {.x = gr->window.top_left.x, .y = f.y},
               (Vector2) {.x = gr->window.top_left.x + grid.x, .y = f.y},
               1, BLACK);

    DrawLineEx((Vector2) {.x = f.x, .y = gr->window.top_left.y},
               (Vector2) {.x = f.x, .y = gr->window.top_left.y + grid.y},
               1, BLACK);
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
