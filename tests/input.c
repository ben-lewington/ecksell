#include <sys/types.h>

#include "../lib/raylib-5.0_linux_amd64/include/raylib.h"

#define INPUT_IMPL
#include "../src/input.h"
#define ARENA_IMPL
#include "../src/arena.h"
#define INTO_ARENA
#include "../src/array.h"
#define LOGGING_IMPL
#include "../src/logging.h"
#include "../src/types.h"

#define TO_VEC2(g) GRID_CAST(g, Vector2)

static bool capture_text = false;

int main(void) {
    InitWindow(800, 600, "Input Test");
    SetTargetFPS(30);
    SetExitKey(KEY_ESCAPE);

    Arena alc = {0};

    float pad = 5;

    Input input = {
        .font_size = 30,
        .font_spacing = 1,
        .text = {0},
    };

    Rectangle boundary = {
        .x      = 100,
        .y      = 100,
        .width  = 100,
        .height = 30,
    };


    // this could be a dynamic array of timedeltas if needed
    float dt = 0;
    while (!WindowShouldClose()) {

        BeginDrawing();
            DrawText(TextFormat("capture_mode: %s", (capture_text) ? "true" : "false"),
                     5, 5, 10, BLACK);
            DrawText(TextFormat("input: %.*s", input.text.len, input.text.inner),
                     5, 20, 10, BLACK);
            ClearBackground(WHITE);
            if (capture_text) {
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
            }

            // draw the bounding rectangle
            DrawRectangleRec(boundary, BLUE);

            Rectangle iboundary = {
                .x      = boundary.x + pad,
                .y      = boundary.y + pad,
                .width  = boundary.width - 2 * pad,
                .height = boundary.height - 2 * pad,
            };

            input_renderInto(&input, iboundary);

        EndDrawing();

        if (IsKeyPressed(KEY_TAB)) capture_text = !capture_text;
    }
}
