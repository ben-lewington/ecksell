#include <assert.h>
#include <stdbool.h>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define GRID_IMPL
#include "../src/grid.h"
#define INPUT_IMPL
#include "../src/input.h"
#define ARRAY_IMPL
#include "../src/array.h"
#define ARENA_IMPL
#include "../src/arena.h"
#define LOGGING_IMPL
#include "../src/logging.h"
#define TYPES_IMPL
#include "../src/types.h"

static Font load_font;
void DrawTextApp(const char *text, Vector2 position, float font_size, float spacing, Color tint) {
    DrawTextEx(load_font, text, position, font_size, spacing, tint);
}

static AppCfg cfg = {
    .pad = 5,
    .grid_item = {
        .x = 50.,
        .y = 20.,
    },
    .curs_speed = {
        .x = 6,
        .y = 4,
    },
};

static KeyboardKey mode_key_transitions[MODES_COUNT][MODES_COUNT] = {
    [MODE_NORMAL] = {
        [MODE_NORMAL] = KEY_NULL,
        [MODE_INSERT] = KEY_I,
        [MODE_FIND]   = KEY_SLASH,
    },
    [MODE_INSERT] = {
        [MODE_NORMAL] = KEY_ESCAPE,
        [MODE_INSERT] = KEY_NULL,
        [MODE_FIND]   = KEY_NULL,
    },
    [MODE_FIND] = {
        [MODE_NORMAL] = KEY_ESCAPE,
        [MODE_INSERT] = KEY_NULL,
        [MODE_FIND]   = KEY_NULL,
    },
};

static Cursor modal_dim = {
    .x = 100,
    .y = 50,
};

static AppState state = {
    .mode = MODE_NORMAL,
    .write_text = DrawTextApp,
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
    };
    log_info("grid render data " GRID_RENDER_FMT_STR, GRID_RENDER_FMT(&gr));

    Arena alc = {0};

    Input input = {
        .font_size = 10,
        .font_spacing = 1,
        {0},
    };

    InitWindow(800, 600, "ZL Sheets");

    load_font = LoadFont("../resources/fonts/JetBrainsMonoNerdFontMono-Regular.ttf");
    state.write_text = DrawTextApp;
    SetExitKey(KEY_END);
    SetTargetFPS(30);

    float dt = 0;
    while (!WindowShouldClose()) {
        state.screen = (Dimensions) {
            .x = GetScreenWidth(),
            .y = GetScreenHeight()
        };

        Index max = grid_getRenderSpan(&gr, &cfg, &state);
        GridMove gm = grid_captureFrameKeys();

        switch (state.mode) {
        case MODE_NORMAL: {
            const Offset lattice = GRID_SCALE(1.5, cfg.grid_item);
            Offset ac = { 0 };

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
                gr.vpointer.x = (max.x - 1) * cfg.grid_item.x - cfg.pad;
                gr.vroot.x += 1;
            }
            if (cur_gi.y >= max.y) {
                gr.vpointer.y = (max.y - 1) * cfg.grid_item.y - cfg.pad;
                gr.vroot.y += 1;
            }
        } break;
        case MODE_INSERT: {
            int key = GetCharPressed();
            while (key > 0) {
                log_debug("keypress %d", key);
                if (key >= 32 && key <= 125) {
                    array_push(&alc, &input.text, (char)key);
                }
                log_debug("s \"%.*s\"", (int) input.text.len, input.text.inner);
                key = GetCharPressed();
            }
            if (IsKeyDown(KEY_BACKSPACE)) {
                if (dt >= 0.08) {
                    input.text.len = (input.text.len == 0) ? 0 : input.text.len - 1;
                    dt = 0;
                }

                log_debug("backspace debounce %f", dt);
                dt += GetFrameTime();
            } else if (IsKeyPressed(KEY_BACKSPACE)) {
                input.text.len = (input.text.len == 0) ? 0 : input.text.len - 1;
                dt = 0;
            }
        } break;
        case MODE_FIND  : {
            int key = GetCharPressed();
            while (key > 0) {
                log_debug("keypress %d", key);
                if (key >= 32 && key <= 125) {
                    array_push(&alc, &input.text, (char)key);
                }
                log_debug("s \"%.*s\"", (int) input.text.len, input.text.inner);
                key = GetCharPressed();
            }
            if (IsKeyDown(KEY_BACKSPACE)) {
                if (dt >= 0.08) {
                    input.text.len = (input.text.len == 0) ? 0 : input.text.len - 1;
                    dt = 0;
                }

                log_debug("backspace debounce %f", dt);
                dt += GetFrameTime();
            } else if (IsKeyPressed(KEY_BACKSPACE)) {
                input.text.len = (input.text.len == 0) ? 0 : input.text.len - 1;
                dt = 0;
            }
        } break;
        default: assert(0 && "unreachable");
        }

        const KeyboardKey *tr = mode_key_transitions[state.mode];
        for (int i = 0; i < MODES_COUNT; ++i) {
            const KeyboardKey k = tr[i];
            if (IsKeyPressed(k) || IsKeyDown(k)) state.mode = (AppMode)i;
        }

        BeginDrawing();
            ClearBackground(WHITE);
            // DrawRectangleRec((Rectangle) { .x =  100,})
            grid_render(&gr, &cfg, &state);

            switch (state.mode) {
            case MODE_NORMAL: {}; break;
            case MODE_INSERT: {
                Rectangle cell_rect = grid_currentCell(&gr, cfg.grid_item);
                input_renderInto(&input, cell_rect);
            }; break;
            case MODE_FIND: {
                Dimensions grid_dims = window_dimensions(gr.window, state.screen);
                Rectangle modal_rect = {
                    .x = gr.window.top_left.x + ((grid_dims.x - modal_dim.x) / 2),
                    .y = gr.window.top_left.y + ((grid_dims.y - modal_dim.y) / 2),
                    .width = modal_dim.x,
                    .height = modal_dim.y,
                };
                DrawRectangleRec(modal_rect, WHITE);
                DrawRectangleLinesEx(modal_rect, 1, BLACK);

                RECT_PAD(modal_rect, cfg.pad);

                input_renderInto(&input, modal_rect);
            }; break;
            default: assert(0 && "unreachable");
            }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
