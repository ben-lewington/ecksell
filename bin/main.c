#include "../lib/raylib-5.0_linux_amd64/include/raylib.h"
#include <stdbool.h>

typedef struct {
  int x, y;
} Index;

typedef struct {
  float x, y;
} Cursor;

typedef struct {
  float x, y;
} Velocity;

typedef struct {
  Index root;
  Cursor offset;
} GridPos;

static Cursor curs = {
    .x = 0,
    .y = 0,
};

static Velocity curs_vel = {
    .x = 0.2,
    .y = 0.3,
};

static Index rel_origin = {
    .x = 0,
    .y = 0,
};

static bool modal = false;
static Cursor modal_dim = {
    .x = 100,
    .y = 50,
};

void renderGrid();

int main(void) {
    InitWindow(800, 600, "ZL Sheets");
    SetTargetFPS(60);
    SetExitKey(KEY_Q);

    Cursor box = {
        .x = 50.,
        .y = 20.,
    };

    float pad = 5;

    while (!WindowShouldClose()) {
        Cursor window = {
            .x = GetScreenWidth(),
            .y = GetScreenHeight(),
        };

        Cursor top_left = {
            .x = 50,
            .y = 50,
        };

        Cursor bottom_inset = {
            .x = 25,
            .y = 25,
        };

        Cursor grid = {
            .x = window.x - top_left.x - bottom_inset.x,
            .y = window.y - top_left.y - bottom_inset.y,
        };

        Index max = {
            .x = (int)(grid.x / box.x),
            .y = (int)(grid.y / box.y),
        };

        BeginDrawing();

        ClearBackground(WHITE);

        Cursor c = curs;
        if (IsKeyDown(KEY_L)) {
            curs.x = curs.x + curs_vel.x;

            if (curs.x >= (float)max.x + rel_origin.x - 1) {
                rel_origin.x = (rel_origin.x < 1) ? 1 : rel_origin.x + 1;
            }
        }

        if (IsKeyDown(KEY_H)) {
            int c = curs.x - curs_vel.x;

            if (c <= (float)rel_origin.x) {
                curs.x = rel_origin.x;
                rel_origin.x = (rel_origin.x == 0) ? 0 : rel_origin.x - 1;
            }

            curs.x = c;
        }
        if (IsKeyDown(KEY_J)) {
            curs.y = curs.y + curs_vel.y;

            if (curs.y >= (float)max.y + rel_origin.y - 1) {
                rel_origin.y = (rel_origin.y < 1) ? 1 : rel_origin.y + 1;
            }
        }

        if (IsKeyDown(KEY_K)) {
            int c = curs.y - curs_vel.y;

            if (c <= (float)rel_origin.y) {
                curs.y = rel_origin.y;
                rel_origin.y = (rel_origin.y == 0) ? 0 : rel_origin.y - 1;
            }

            curs.y = c;
        }
        //
        // if (IsKeyDown(KEY_J)) {
        //   curs.y += curs_vel.y;
        //   if (curs.y >= (float)max.y + rel_origin.y) {
        //     rel_origin.y += 1;
        //   }
        // }
        //
        // if (IsKeyDown(KEY_K)) {
        //   curs.y = curs.y - curs_vel.y;
        //
        //   if (curs.y < 1.)
        //     curs.y = 1;
        //
        //   if (curs.y <= rel_origin.y) {
        //     rel_origin.y -= 1;
        //   }
        // }
        //
        if (IsKeyPressed(KEY_F) && !modal) modal = true;

        if (IsKeyPressed(KEY_ESCAPE) && modal) modal = false;

        for (int xi = 0; xi < max.x; ++xi) {
          float box_xi = top_left.x + xi * box.x;
          DrawLine(box_xi, top_left.y, box_xi, window.y - bottom_inset.y, BLACK);
          for (int yi = 0; yi < max.y; ++yi) {
            float box_yi = top_left.y + yi * box.y;
            DrawLine(top_left.x, box_yi, window.x - bottom_inset.x, box_yi, BLACK);
            if (yi == 0)
              continue;
            DrawText(TextFormat("%d", rel_origin.y + yi - 1), top_left.x + pad,
                     box_yi + pad, 9, BLACK);
            if (xi == 0)
              continue;
            if ((int)curs.x == rel_origin.x + xi - 1 &&
                (int)curs.y == rel_origin.y + yi - 1) {
              if (!modal)
                DrawRectangleLinesEx(
                    (Rectangle){
                        .x = box_xi,
                        .y = box_yi,
                        .width = box.x,
                        .height = box.y,
                    },
                    pad / 2, GREEN);
              DrawText(TextFormat("(%d, %d)", rel_origin.x + xi - 1,
                                  rel_origin.y + yi - 1),
                       top_left.x + pad, top_left.y + pad, 9, BLACK);
            }
          }
          if (xi == 0)
            continue;

          DrawText(TextFormat("%d", rel_origin.x + xi - 1), box_xi + pad,
                   top_left.y + pad, 10, BLACK);
        }
        int fx = top_left.x + max.x * box.x;
        int fy = top_left.y + max.y * box.y;
        DrawLine(top_left.x, fy, window.x - bottom_inset.x, fy, BLACK);
        DrawLine(fx, top_left.y, fx, window.y - bottom_inset.y, BLACK);

        DrawRectangle(1, 0, window.x, top_left.y, GRAY);
        DrawRectangle(0, 0, top_left.x, window.y, GRAY);
        DrawRectangle(0, window.y - bottom_inset.y, window.x, window.y, GRAY);
        DrawRectangle(window.x - bottom_inset.x, 0, window.x, window.y, GRAY);

        if (modal)
          DrawRectangle(top_left.x + ((grid.x - modal_dim.x) / 2),
                        top_left.y + ((grid.y - modal_dim.y) / 2), modal_dim.x,
                        modal_dim.y, RED);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
