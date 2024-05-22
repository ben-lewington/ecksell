#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include "../lib/raylib-5.0_linux_amd64/include/raylib.h"

#include "arena.h"
#define INTO_ARENA
#include "array.h"
#include "types.h"

#define TO_VEC2(g) GRID_CAST(g, Vector2)

typedef struct {
    float  font_size;
    float  font_spacing;
    String text;
} Input;

void input_render(const Input *u);

#endif // !INPUT_H
#define INPUT_IMPL
#ifdef INPUT_IMPL

void input_renderInto(const Input *input, Rectangle boundary) {
    float font_sz = input->font_size;
    if (boundary.height < font_sz) font_sz = boundary.height;
    const String s = input->text;
    if (s.len == 0) return;

    Vector2 f = MeasureTextEx(GetFontDefault(), str, font_sz, input->font_spacing);
    const char *str = TextFormat("%.*s", s.len, input->text.inner);
    while (f.x >= boundary.width) {
        str++;
        f = MeasureTextEx(GetFontDefault(), str, font_sz, input->font_spacing);
    }
    // const Dimensions rec_topleft = GRID_CAST(boundary, )
    DrawTextEx(GetFontDefault(), str, GRID_CAST(boundary, Vector2), font_sz, input->font_spacing, BLACK);
}

#endif // INPUT_IMPL
