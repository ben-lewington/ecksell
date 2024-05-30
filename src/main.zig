const std = @import("std");

const raylib = @import("raylib.zig");
const rl = raylib.include();

const app = @import("app.zig");
const grid = @import("grid.zig");
const Storage = @import("Storage.zig");

const Coord = grid.Coord;
const Key = raylib.KeyboardKey;
const Rect = grid.area.Rect;
const @"0" = std.mem.zeroes;
const IxStr = grid.AlphabetIx("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
// const IxStr = Grid.IndexStr("AB");

pub const std_options = .{ .log_level = .info };

var state: app.State = .{
    .mode = .normal,
    .screen = undefined,
    .current_input = @"0"(Rect(f32)),
};

var cfg: app.Config = .{
    .pad = 2,
    .grid_item = .{ .x = 50.0, .y = 20.0 },
    .modal_dim = .{ .x = 50.0, .y = 20.0 },
    .curs_velocity = .{ .x = 6.0, .y = 4.0 },
};

pub fn main() !void {
    std.log.info("\n", .{});

    const Dir = enum { up, down, left, right };
    const GridRect = grid.Render;

    var grid_state: GridRect = .{
        .position = @"0"(GridRect.Pos),
        .grid_item = cfg.grid_item,
        .window = @"0"(Rect(f32)),
    };

    // "big" chunks of memory
    const page_alc = std.heap.page_allocator;

    // General purpose arena.
    var arena = std.heap.ArenaAllocator.init(page_alc);
    defer _ = arena.reset(.free_all);

    // user input arena.
    var input = std.ArrayList(u8).init(arena.allocator());
    try input.ensureTotalCapacity(50);

    var storage: Storage = .{
        .store = .{
            .cells = try std.ArrayList(grid.Cell(Storage.Field.Value)).initCapacity(arena.allocator(), 1),
            .text = std.ArrayList(u8).init(arena.allocator()),
            .reals = std.ArrayList(f32).init(page_alc),
            .indices = std.ArrayList(usize).init(page_alc),
            .integers = std.ArrayList(isize).init(page_alc),
        },
        .boundary = .{
            .top_left = @"0"(Coord(isize)),
            .bottom_right = @"0"(Coord(isize)),
        },
        .regions = std.ArrayList(Storage.Heap.Region.Value).init(arena.allocator()),
    };
    // rl.SetConfigFlags(rl.FLAG_WINDOW_RESIZABLE | rl.FLAG_WINDOW_MAXIMIZED);
    rl.InitWindow(800, 600, "ZL Sheets");
    rl.GuiSetFont(rl.LoadFontEx("./resources/fonts/homespun.ttf", 10, null, 0));
    rl.SetTargetFPS(30);
    rl.SetExitKey(Key.end.literally());

    var dt: f32 = 0;
    var current_cell: ?Storage.Field.Value = null;
    while (!rl.WindowShouldClose()) {
        state.screen = .{
            .x = @floatFromInt(rl.GetScreenWidth()),
            .y = @floatFromInt(rl.GetScreenHeight()),
        };

        grid_state.window = .{
            .x = 5,
            .y = 5,
            .width = state.screen.x - 5 - 4,
            .height = state.screen.y - cfg.grid_item.y - 5 - 4,
        };

        const cur = grid_state.currentCell();
        const cursor_ix = grid_state.currentCursorIndex();
        const grid_ix = Coord(isize).binop(.add, cursor_ix, grid_state.position.root);
        current_cell = storage.getValueAt(grid_ix);

        if (state.mode.shouldTransition()) |mode| {
            state.mode = mode;
            input.items.len = 0;
        } else switch (state.mode) {
            .normal => {
                const grid_move = Key.CaptureGroup(Dir, .{
                    .unique_keys = &[_]Key{ .k, .j, .h, .l },
                    .modifier = .null,
                }).getFrameKeys();

                var ac: Coord(f32) = .{ .x = 0, .y = 0 };

                if (grid_move.isKeySet(.up)) ac.y -= cfg.curs_velocity.y;
                if (grid_move.isKeySet(.down)) ac.y += cfg.curs_velocity.y;
                if (grid_move.isKeySet(.left)) ac.x -= cfg.curs_velocity.x;
                if (grid_move.isKeySet(.right)) ac.x += cfg.curs_velocity.x;

                ac.ibinop(.mul, Coord(f32).shear(60 * rl.GetFrameTime()));

                grid_state.moveCursor(ac, cfg.pad);
            },
            .insert, .find => {
                var key = rl.GetCharPressed();
                while (key > 0) {
                    if (key >= 32 and key <= 125) try input.append(@intCast(key));
                    key = rl.GetCharPressed();
                }
                if (input.items.len > 0) {
                    const break_insert = Key.CaptureGroup(Dir, .{
                        .unique_keys = &[_]Key{ .enter, .tab },
                        .modifier = .left_shift,
                    }).getFrameKeys();

                    if (break_insert.inner > 0) {
                        _ = storage.storeValueAt(grid_ix, input.items);

                        var thr = std.mem.zeroes(Coord(isize));
                        if (break_insert.isKeySet(.up)) thr.y += 1;
                        if (break_insert.isKeySet(.down)) thr.y -= 1;
                        if (break_insert.isKeySet(.left)) thr.x += 1;
                        if (break_insert.isKeySet(.right)) thr.x -= 1;

                        grid_state.position.offset = Coord(f32)
                            .binopChain(.add, thr, cursor_ix)
                            .then(.mul, cfg.grid_item)
                            .finally(.add, grid_state.window);

                        state.mode = .normal;
                    } else if (rl.IsKeyDown(Key.backspace.literally())) {
                        if (dt >= 0.08) {
                            input.items.len = input.items.len - 1;
                            dt = 0;
                        }
                        dt += rl.GetFrameTime();
                    } else if (rl.IsKeyPressed(Key.backspace.literally())) {
                        input.items.len = input.items.len - 1;
                        dt = 0;
                    }
                }
            },
        }

        std.log.debug("{}", .{storage.store});

        rl.BeginDrawing();
        rl.ClearBackground(rl.WHITE);
        rl.GuiDrawRectangle(grid_state.window.as(), 1, rl.BLACK, rl.WHITE);
        // _ = rl.GuiStatusBar(.{ .x = 0, .y = 0, .width = state.screen.x, .height = 20 }, "Hello");
        var it = grid_state.iter();
        while (it.next()) |grid_cell| {
            const ix = grid_cell.ix;
            const rect = grid_cell.rect;

            const rc = Coord(isize)
                .binopChain(.add, ix, grid_state.position.root)
                .finally(.sub, Coord(isize).shear(1));

            if (ix.x == 0 or ix.y == 0) {
                if (ix.x == 0 and ix.y == 0) {
                    rl.GuiDrawRectangle(
                        .{
                            .x = grid_state.window.x,
                            .y = grid_state.window.y,
                            .width = cfg.grid_item.x,
                            .height = grid_state.window.height,
                        },
                        1,
                        rl.BLACK,
                        rl.Fade(rl.SKYBLUE, 0.2),
                    );
                    rl.GuiDrawRectangle(
                        .{
                            .x = grid_state.window.x,
                            .y = grid_state.window.y,
                            .width = grid_state.window.width,
                            .height = cfg.grid_item.y,
                        },
                        1,
                        rl.BLACK,
                        rl.Fade(rl.SKYBLUE, 0.2),
                    );
                    const cur_col = IxStr.convert(if (grid_ix.x < 0) 0 else @as(usize, @intCast(grid_ix.x)));
                    rl.GuiDrawText(rl.TextFormat("%.*s%d", cur_col.len, cur_col.buf[0..cur_col.len].ptr, grid_ix.y), rect.as(), rl.TEXT_ALIGN_CENTER, rl.BLACK);
                }

                const col = IxStr.convert(if (rc.x < 0) 0 else @as(usize, @intCast(rc.x)));
                if (ix.x > 0) rl.GuiDrawText(rl.TextFormat("%.*s", col.len, col.buf[0..col.len].ptr), rect.as(), rl.TEXT_ALIGN_CENTER, rl.BLACK);
                if (ix.y > 0) rl.GuiDrawText(rl.TextFormat("%d", rc.y), rect.as(), rl.TEXT_ALIGN_CENTER, rl.BLACK);

                if (ix.x == 0) {
                    rl.DrawLineEx(.{ .x = grid_state.window.x, .y = rect.y + rect.height }, .{
                        .x = grid_state.window.x + grid_state.window.width,
                        .y = rect.y + rect.height,
                    }, 1, rl.BLACK);
                }

                if (ix.y == 0) {
                    rl.DrawLineEx(.{ .x = rect.x + rect.width, .y = grid_state.window.y }, .{
                        .x = rect.x + rect.width,
                        .y = grid_state.window.y + grid_state.window.height,
                    }, 1, rl.BLACK);
                }
            } else if (storage.hasData()) {
                if (storage.getValueAt(rc)) |v| {
                    if (!Coord(isize).equ(ix, Coord(isize).binop(
                        .add,
                        cursor_ix,
                        Coord(isize).shear(1),
                    )) or state.mode != .insert) {
                        rl.GuiDrawText(storage.guiTextFormat(v), rect.as(), rl.TEXT_ALIGN_CENTER, rl.BLACK);
                    }
                    rl.DrawRectangleRec(
                        .{ .x = rect.x + 0.5, .y = rect.y + 0.5, .width = 1, .height = 10 },
                        // 2,
                        Storage.Field.pallette[@intFromEnum(v)],
                    );
                }
            }
        }

        const mode_lbl = .{
            .x = grid_state.window.x,
            .y = grid_state.window.y + grid_state.window.height - 1,
            .width = cfg.grid_item.x,
            .height = cfg.grid_item.y,
        };
        const input_bar = .{
            .x = grid_state.window.x + cfg.grid_item.x - 1,
            .y = grid_state.window.y + grid_state.window.height - 1,
            .width = grid_state.window.width - cfg.grid_item.x + 1,
            .height = cfg.grid_item.y,
        };
        rl.GuiDrawRectangle(mode_lbl, 1, rl.BLACK, app.Mode.pallette[@intFromEnum(state.mode)]);

        _ = rl.GuiDrawText(@constCast(@ptrCast(@tagName(state.mode))), mode_lbl, 1, rl.BLACK);
        rl.GuiDrawRectangle(input_bar, 1, rl.BLACK, rl.WHITE);

        switch (state.mode) {
            .normal => {
                rl.DrawRectangleLinesEx(.{
                    .x = cur.x,
                    .y = cur.y,
                    .width = cur.width - 0.5,
                    .height = cur.height - 0.5,
                }, 2, rl.GREEN);

                if (current_cell) |c| {
                    rl.GuiDrawText(storage.guiTextFormat(c), input_bar, 1, rl.BLACK);
                }
            },
            .insert => {
                rl.DrawRectangleLinesEx(.{
                    .x = cur.x,
                    .y = cur.y,
                    .width = cur.width - 0.5,
                    .height = cur.height - 0.5,
                }, 2, rl.GREEN);

                // try input.append();
                if (input.items.len > 0) {
                    const s = rl.TextFormat("%.*s", input.items.len, input.items.ptr);
                    rl.GuiDrawText(s, cur.as(), 1, rl.BLACK);
                    rl.GuiDrawText(s, input_bar, 1, rl.BLACK);
                }
            },
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
