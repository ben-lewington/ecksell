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

typedef struct  {
    Index  vroot;
    Cursor vpointer;
    Window window;
} GridRender;

#define GRID_RENDER_FMT_STR                        \
    "{\n"                                          \
    "    vroot = (" GRID_FMT_STRI "),\n"           \
    "    vpointer = (" GRID_FMT_STRF "),\n"        \
    "    window_top_left = (" GRID_FMT_STRF "),\n" \
    "}"

#define GRID_RENDER_FMT(gr) \
    GRID_PAIRS((gr)->vroot), GRID_PAIRS((gr)->vpointer), GRID_PAIRS((gr)->window.top_left)

void      grid_render(const GridRender *gr, const AppCfg *cfg, const AppState *state);
Index     grid_currentIndex(const GridRender *gr, const Dimensions grid_item);
Rectangle grid_currentCell(const GridRender *gr, const Dimensions grid_item);
Index     grid_getRenderSpan(const GridRender *gr, const AppCfg *cfg, const AppState *state);
GridMove  grid_captureFrameKeys(void);

#endif // !GRID_H
#define GRID_IMPL
#ifdef GRID_IMPL

Index grid_currentIndex(const GridRender *gr, const Dimensions grid_item) {
    Offset lattice = GRID_SCALE(1.5, grid_item);
    Cursor lp = GRID_ADD(gr->vpointer, lattice);

    return (Index) {
        .x = (int)(lp.x / grid_item.x) - 1,
        .y = (int)(lp.y / grid_item.y) - 1,
    };
}

Rectangle grid_currentCell(const GridRender *gr, const Dimensions grid_item) {
    Offset lattice = GRID_SCALE(1.5, grid_item);
    Cursor lp = GRID_ADD(gr->vpointer, lattice);
    Index lpi = GRID_DIV(lp, grid_item);
    lp = (Cursor)GRID_MUL(lpi, grid_item);


    return (Rectangle) {
        .x = lp.x + gr->window.top_left.x,
        .y = lp.y + gr->window.top_left.y,
        .width = grid_item.x,
        .height = grid_item.y,
    };
}

Index grid_getRenderSpan(const GridRender *gr, const AppCfg *cfg, const AppState *state) {
    Dimensions grid = window_dimensions(gr->window, state->screen);
    return (Index) {
        .x = (int)(grid.x / cfg->grid_item.x) - 1,
        .y = (int)(grid.y / cfg->grid_item.y) - 1,
    };
}

void grid_render(const GridRender *gr, const AppCfg *cfg, const AppState *state) {
    Offset     lattice = GRID_SCALE(1.5, cfg->grid_item);
    Index      max = grid_getRenderSpan(gr, cfg, state);
    Dimensions grid = window_dimensions(gr->window, state->screen);
    DrawRectangleLines(GRID_PAIRS(gr->window.top_left), GRID_PAIRS(grid), BLACK);
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
                Cursor gpad = GRID_SHEAR(gr->window.top_left, cfg->pad);
                Index cur_ix = GRID_ADD(grid_currentIndex(gr, cfg->grid_item), gr->vroot);
                (state->write_text)(TextFormat("%d, %d", GRID_PAIRS(cur_ix)),
                         GRID_CAST(gpad, Vector2), 10, 1, BLACK);

            }
            if (xi > 0)
                (state->write_text)(TextFormat("%d", gr->vroot.x + xi - 1),
                        (Vector2) GRID_NEW(it_pos.x + cfg->pad, gr->window.top_left.x + cfg->pad), 10, 1, BLACK);
            if (yi > 0)
                (state->write_text)(TextFormat("%d", gr->vroot.y + yi - 1),
                        (Vector2) GRID_NEW(gr->window.top_left.y + cfg->pad, it_pos.y + cfg->pad), 10, 1, BLACK);
        }

        if (xi == 0)
            DrawLineEx((Vector2) { .x = gr->window.top_left.x,          .y = it_pos.y },
                       (Vector2) { .x = gr->window.top_left.x + grid.x, .y = it_pos.y },
                       1, BLACK);

        DrawLineEx((Vector2) { .x = it_pos.x, .y = gr->window.top_left.y },
                   (Vector2) { .x = it_pos.x, .y = gr->window.top_left.y + grid.y },
                   1, BLACK);

        Cursor lp = GRID_ADD(gr->vpointer, lattice);
        Index lpi = {
            .x = (int)(lp.x / cfg->grid_item.x) - 1,
            .y = (int)(lp.y / cfg->grid_item.y) - 1,
        };
        Index gid = GRID_SHEAR(it, -1);
        // TODO(BL): hoist the state logic up to the main event loop, have a grid_renderSelected
        // function
        if (GRID_EQU(lpi, gid)) {
            if (state->mode == MODE_NORMAL) DrawRectangleLinesEx((Rectangle) {
                .x = it_pos.x,
                .y = it_pos.y,
                .width = cfg->grid_item.x,
                .height = cfg->grid_item.y,
            }, cfg->pad / 2., GREEN);
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
