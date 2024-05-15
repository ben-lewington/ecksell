#include <stdbool.h>

#include "../lib/raylib-5.0_linux_amd64/include/raylib.h"

#define GRID_IMPL
#include "../src/grid.h"
#define INPUT_IMPL
#include "../src/input.h"
#define LOGGING_IMPL
#include "../src/logging.h"

#define screen_dims

static AppCfg cfg = {
    .grid_pad = 5,
    .grid_item = {
        .x = 50.,
        .y = 20.,
    },
    .curs_speed = {
        .x = 6,
        .y = 4,
    },
};

static bool modal = false;
static Cursor modal_dim = {
    .x = 100,
    .y = 50,
};

int main(void) {
    GridRender gr = {
        .window = {
            .top_left = {
                .x = 50,
                .y = 50,
            },
            .bottom_inset = {
                .x = 25,
                .y = 25,
            }
        },
        .vpointer = {0},
        .vroot = {0},
    };

    InitWindow(800, 600, "ZL Sheets");
    SetTargetFPS(30);
    SetExitKey(KEY_Q);

    Dimensions screen;
    while (!WindowShouldClose()) {
        screen = (Dimensions) {
            .x = GetScreenWidth(),
            .y = GetScreenHeight()
        };
        Dimensions grid = grid_windowSize(gr.window, screen);
        Index max = {
            .x = (int)(grid.x / cfg.grid_item.x) - 1,
            .y = (int)(grid.y / cfg.grid_item.y) - 1,
        };

        GridMove gm = grid_captureFrameKeys();

        // if (!modal) grid_advanceCursor(gm, &gr);
        if (!modal) {
            const Offset lattice = GRID_SCALE(1.5, cfg.grid_item);
            Offset ac = {0};

            if (gm.inner & GRID_MOVE_UP)    ac.y -= cfg.curs_speed.y;
            if (gm.inner & GRID_MOVE_RIGHT) ac.x += cfg.curs_speed.x;
            if (gm.inner & GRID_MOVE_LEFT)  ac.x -= cfg.curs_speed.x;
            if (gm.inner & GRID_MOVE_DOWN)  ac.y += cfg.curs_speed.y;

            ac = (Offset)GRID_SCALE(GetFrameTime() * 60, ac);
            Cursor np = GRID_ADD(gr.vpointer, ac);
            np = (Cursor)GRID_ADD(np, lattice);

            gr.vpointer = (Cursor)GRID_ADD(gr.vpointer, ac);
            Index cur_gi = grid_currentIndex(&gr, cfg.grid_item);
            if (cur_gi.x < 0) {
                gr.vpointer.x = 0;
                gr.vroot.x = (gr.vroot.x - 1 < 0) ? 0 : gr.vroot.x - 1;
            }
            if (cur_gi.y < 0) {
                gr.vpointer.y = 0;
                gr.vroot.y = (gr.vroot.y - 1 < 0) ? 0 : gr.vroot.y - 1;
            }
            if (cur_gi.x >= max.x) {
                gr.vpointer.x = (max.x - 1) * cfg.grid_item.x - cfg.grid_pad;
                gr.vroot.x += 1;
            }
            if (cur_gi.y >= max.y) {
                gr.vpointer.y = (max.y - 1) * cfg.grid_item.y - cfg.grid_pad;
                gr.vroot.y += 1;
            }
        }


        if (IsKeyPressed(KEY_F) && !modal) modal = true;
        if (IsKeyPressed(KEY_ESCAPE) && modal) modal = false;

        BeginDrawing();
            ClearBackground(WHITE);
            grid_render(&gr, screen, &cfg, modal);

            if (modal) {
                Dimensions grid_dims = grid_windowSize(gr.window, screen);
                Rectangle modal_rect = {
                    .x = gr.window.top_left.x + ((grid_dims.x - modal_dim.x) / 2),
                    .y = gr.window.top_left.y + ((grid_dims.y - modal_dim.y) / 2),
                    .width = modal_dim.x,
                    .height = modal_dim.y,
                };
                DrawRectangleRec(modal_rect, RED);
            }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
