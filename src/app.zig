pub const rl = @cImport({
    @cInclude("raylib.h");
    @cDefine("RAYGUI_IMPLEMENTATION", {});
    @cInclude("raygui.h");
});

const Grid = @import("grid.zig");
const keys = @import("keys.zig");

const Coord = Grid.Coord;
const Key = keys.KeyboardKey;

pub const Mode = enum {
    normal,
    insert,
    find,

    const n = @typeInfo(Mode).Enum.fields.len;
    pub const mode_key_transitions: [n][n]Key = [_][n]Key{
        [_]Key{ .null, .i, .slash },
        [_]Key{ .escape, .null, .null },
        [_]Key{ .escape, .null, .null },
    };

    pub const pallette: [n]rl.Color = [_]rl.Color{ rl.SKYBLUE, rl.PINK, rl.GREEN };
};

pub const State = struct {
    mode: Mode,
    screen: Coord(f32),
};

pub const Config = struct {
    pad: f32,
    grid_item: Coord(f32),
    modal_dim: Coord(f32),
    curs_velocity: Coord(f32),
};

pub const ScreenView = struct {
    top_left: Coord(f32),
    bottom_inset: Coord(f32),

    pub fn toRect(self: @This(), screen: Grid.Coord(f32)) rl.Rectangle {
        return .{
            .x = self.top_left.x,
            .y = self.top_left.y,
            .width = screen.x - (self.top_left.x + self.bottom_inset.x),
            .height = screen.y - (self.top_left.y + self.bottom_inset.y),
        };
    }
};
