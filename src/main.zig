const std = @import("std");

pub const rl = @cImport({
    @cInclude("raylib.h");
    @cDefine("RAYGUI_IMPLEMENTATION", {});
    @cInclude("raygui.h");
});

const app = @import("app.zig");
const Grid = @import("grid.zig");
const keys = @import("keys.zig");
const Key = keys.KeyboardKey;

const Coord = Grid.Coord;

pub const std_options = .{
    .log_level = .info,
};

var state: app.State = .{
    .mode = .normal,
    .screen = undefined,
};

var cfg: app.Config = .{
    .pad = 5,
    .grid_item = .{
        .x = 50.0,
        .y = 20.0,
    },
    .modal_dim = .{
        .x = 50.0,
        .y = 20.0,
    },
    .curs_velocity = .{
        .x = 6.0,
        .y = 4.0,
    },
};

pub fn main() !void {
    const grid_screen: app.ScreenView = .{
        .top_left = .{ .x = 50, .y = 50 },
        .bottom_inset = .{ .x = 25, .y = 25 },
    };

    var gview: Grid.View = .{
        .inner = .{
            .virt_offset = .{ .x = 0, .y = 0 },
            .virt_root = .{ .x = 0, .y = 0 },
        },
        .grid_item = cfg.grid_item,
        .window = .{ .x = 0, .y = 0, .width = 0, .height = 0 },
    };
    var alc = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer _ = alc.reset(.free_all);

    var input = std.ArrayList(u8).init(alc.allocator());

    rl.SetConfigFlags(rl.FLAG_WINDOW_RESIZABLE);
    rl.InitWindow(800, 600, "ZL Sheets");
    rl.SetTargetFPS(30);
    rl.SetExitKey(Key.end.literally());

    var dt: f32 = 0;
    while (!rl.WindowShouldClose()) {
        state.screen = .{
            .x = @floatFromInt(rl.GetScreenWidth()),
            .y = @floatFromInt(rl.GetScreenHeight()),
        };

        gview.window = grid_screen.toRect(state.screen);

        std.log.debug("{}", .{gview});

        switch (state.mode) {
            .normal => {
                const Dir = enum { up, down, left, right };
                const grid_move = Key.CaptureGroup(Dir, &[_]Key{ .k, .j, .h, .l }).getFrameKeys();

                var ac: Coord(f32) = .{ .x = 0, .y = 0 };

                if (grid_move.isKeySet(.up)) ac.y -= cfg.curs_velocity.y;
                if (grid_move.isKeySet(.down)) ac.y += cfg.curs_velocity.y;
                if (grid_move.isKeySet(.left)) ac.x -= cfg.curs_velocity.x;
                if (grid_move.isKeySet(.right)) ac.x += cfg.curs_velocity.x;

                const ft = 60 * rl.GetFrameTime();
                ac = .{
                    .x = ft * ac.x,
                    .y = ft * ac.y,
                };

                std.log.debug("offset {}", .{ac});
                gview.moveCursor(ac, cfg.pad);
                std.log.debug("total offset {}", .{gview.inner.virt_offset});
            },
            .insert => {
                var key = rl.GetCharPressed();
                while (key > 0) {
                    if (key >= 32 and key <= 125) try input.append(@intCast(key));
                    key = rl.GetCharPressed();
                }
                if (input.items.len > 0) {
                    if (rl.IsKeyDown(@intFromEnum(Key.backspace))) {
                        if (dt >= 0.08) {
                            input.items.len = input.items.len - 1;
                            dt = 0;
                        }
                        dt += rl.GetFrameTime();
                    } else if (rl.IsKeyPressed(@intFromEnum(Key.backspace))) {
                        input.items.len = input.items.len - 1;
                        dt = 0;
                    }
                }
            },
            .find => {
                var key = rl.GetCharPressed();
                while (key > 0) {
                    if (key >= 32 and key <= 125) try input.append(@intCast(key));
                    key = rl.GetCharPressed();
                }
                if (input.items.len > 0) {
                    if (rl.IsKeyDown(@intFromEnum(Key.backspace))) {
                        if (dt >= 0.08) {
                            input.items.len = input.items.len - 1;
                            dt = 0;
                        }
                        dt += rl.GetFrameTime();
                    } else if (rl.IsKeyPressed(@intFromEnum(Key.backspace))) {
                        input.items.len = input.items.len - 1;
                        dt = 0;
                    }
                }
            },
        }

        const tr_keys: [3]Key = app.Mode.mode_key_transitions[@intFromEnum(state.mode)];
        for (tr_keys, 0..) |k, i| {
            if (rl.IsKeyPressed(@intFromEnum(k)) or rl.IsKeyDown(@intFromEnum(k))) state.mode = @enumFromInt(i);
        }

        rl.BeginDrawing();
        rl.ClearBackground(rl.WHITE);
        var it = gview.iter();
        rl.GuiDrawRectangle(gview.window, 1, rl.BLUE, rl.WHITE);
        const cursor_ix = gview.currentCursorIndex();
        const cursor_grid_ix = Coord(isize).binop(.add, cursor_ix, gview.inner.virt_root);
        while (it.next()) |grid_cell| {
            const ix = grid_cell.ix;
            const rect = grid_cell.rect;
            if (ix.x == 0 or ix.y == 0) {
                if (ix.x == 0 and ix.y == 0) {
                    rl.GuiDrawText(
                        rl.TextFormat("%d, %d", cursor_grid_ix.x, cursor_grid_ix.y),
                        rect,
                        rl.TEXT_ALIGN_CENTER,
                        rl.BLACK,
                    );
                }

                const rc = Coord(isize).binop(.add, ix, gview.inner.virt_root);
                if (ix.x > 0)
                    rl.GuiDrawText(rl.TextFormat("%d", rc.x), rect, rl.TEXT_ALIGN_CENTER, rl.BLACK);
                if (ix.y > 0)
                    rl.GuiDrawText(rl.TextFormat("%d", rc.y), rect, rl.TEXT_ALIGN_CENTER, rl.BLACK);

                if (ix.x == 0) {
                    rl.DrawLineEx(
                        .{ .x = gview.window.x, .y = rect.y + rect.height },
                        .{ .x = gview.window.x + gview.window.width, .y = rect.y + rect.height },
                        1,
                        rl.BLACK,
                    );
                }

                if (ix.y == 0) {
                    rl.DrawLineEx(
                        .{ .x = rect.x + rect.width, .y = gview.window.y },
                        .{ .x = rect.x + rect.width, .y = gview.window.y + gview.window.height },
                        1,
                        rl.BLACK,
                    );
                }
            }
        }

        rl.DrawRectangleRec(.{ .x = 50, .y = 10, .width = 75, .height = 25 }, app.Mode.pallette[@intFromEnum(state.mode)]);
        rl.GuiDrawText(@constCast(@ptrCast(@tagName(state.mode))), .{ .x = 50, .y = 10, .width = 75, .height = 25 }, 1, rl.BLACK);

        switch (state.mode) {
            .normal => {
                const cur = gview.currentCell();
                const p: f32 = cfg.pad;
                rl.DrawRectangleLinesEx(cur, p / 2.0, rl.GREEN);
            },
            .insert => {},
            .find => {
                // const modal_topleft = {
                //     .x = gr.window.top_left.x + ((grid_dims.x - modal_dim.x) / 2),
                //     .y = gr.window.top_left.y + ((grid_dims.y - modal_dim.y) / 2),
                // };
                // .width = modal_dim.x,
                // .height = modal_dim.y,
            },
        }
        rl.EndDrawing();
    }
}
