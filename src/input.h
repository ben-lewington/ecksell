#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include "../lib/raylib-5.0_linux_amd64/include/raylib.h"

#include "arena.h"
#define INTO_ARENA
#include "array.h"
#include "types.h"
#include "logging.h"

#define TO_VEC2(g) GRID_CAST(g, Vector2)

typedef struct {
    Coord top_left;
    float width;
    float font_size;
    float spacing;
    float pad;
} InputWindow;

typedef struct {
    InputWindow window;
    String text;
} Input;

void input_render(const Input *input, Dimensions screen);

#endif // !INPUT_H
// #define INPUT_IMPL
#ifdef INPUT_IMPL

void input_render(const Input *input, Dimensions screen) {
    const InputWindow w = input->window;
    const String      s = input->text;

    const char *str = TextFormat("%.*s", s.len, input->text.inner);

    // draw the bounding rectangle
    DrawRectangleV(TO_VEC2(w.top_left), (Vector2) {
        .x = w.width + 2 * w.pad,
        .y = w.font_size + 2 * w.pad
    }, BLUE);

    Vector2 f = MeasureTextEx(GetFontDefault(), str, w.font_size, input->window.spacing);

    if (f.x <= 0) return;

    while (f.x >= w.width) {
        str++;
        f = MeasureTextEx(GetFontDefault(), str, w.font_size, input->window.spacing);
    }

    DrawTextEx(GetFontDefault(), str, (Vector2) GRID_SHEAR(w.top_left, w.pad), w.font_size,
        input->window.spacing, BLACK);

}

#endif // INPUT_IMPL
