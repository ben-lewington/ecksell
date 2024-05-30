const raylib = @import("raylib.zig");
const rl = raylib.include();

const grid = @import("grid.zig");
const Key = raylib.KeyboardKey;

const Coord = grid.Coord;

pub const Mode = enum {
    normal,
    insert,
    find,

    const n = @typeInfo(Mode).Enum.fields.len;
    pub const mode_transition_keys: [n][n]Key = [_][n]Key{
        [_]Key{ .null, .i, .slash },
        [_]Key{ .escape, .null, .null },
        [_]Key{ .escape, .null, .null },
    };

    pub const pallette: [n]rl.Color = [_]rl.Color{ rl.SKYBLUE, rl.PINK, rl.GREEN };

    pub fn shouldTransition(mode: @This()) ?@This() {
        const tr_keys: [3]Key = mode_transition_keys[@intFromEnum(mode)];
        for (tr_keys, 0..) |k, i| {
            if (rl.IsKeyPressed(k.literally()) or rl.IsKeyDown(k.literally())) return @enumFromInt(i);
        }
        return null;
    }
};

pub const State = struct {
    mode: Mode,
    screen: Coord(f32),
    current_input: grid.area.Rect(f32),
};

pub const Config = struct {
    pad: f32,
    grid_item: Coord(f32),
    modal_dim: Coord(f32),
    curs_velocity: Coord(f32),
};
