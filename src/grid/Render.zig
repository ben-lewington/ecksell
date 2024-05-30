const std = @import("std");
const C = @import("coord.zig").Coord;
const Rect = @import("2d.zig").Rect;
position: Pos,
window: Rect(f32),
grid_item: C(f32),

pub const Pos = struct {
    root: C(isize),
    offset: C(f32),
};

const VSelf = @This();

pub fn max(self: *const VSelf) C(isize) {
    return .{
        .x = @intFromFloat(@divFloor(self.window.width, self.grid_item.x)),
        .y = @intFromFloat(@divFloor(self.window.height, self.grid_item.y)),
    };
}

pub fn asCursorIndex(grid_item: C(f32), position: C(f32)) C(isize) {
    const lattice = C(f32)
        .binopChain(.mul, C(f32).shear(1.5), grid_item)
        .then(.add, position)
        .finally(.div, grid_item);

    return C(isize).binop(.sub, lattice, C(isize).shear(1));
}

pub fn currentCursorIndex(self: *const VSelf) C(isize) {
    return asCursorIndex(self.grid_item, self.position.offset);
}

pub fn currentCell(self: *const VSelf) Rect(f32) {
    const lattice = .{
        .x = 1.5 * self.grid_item.x + self.position.offset.x,
        .y = 1.5 * self.grid_item.y + self.position.offset.y,
    };

    return .{
        .x = self.window.x + self.grid_item.x * std.math.floor(lattice.x / self.grid_item.x),
        .y = self.window.y + self.grid_item.y * std.math.floor(lattice.y / self.grid_item.y),
        .width = self.grid_item.x,
        .height = self.grid_item.y,
    };
}

pub fn moveCursor(self: *VSelf, offset: C(f32), pad: f32) void {
    std.log.debug("offset {}", .{offset});
    const imax = self.max();
    if (offset.x == 0 and offset.y == 0) return;

    self.position.offset.ibinop(.add, offset);

    const cur_gi = self.currentCursorIndex();
    if (cur_gi.x < 0) {
        self.position.offset.x = 0;
        self.position.root.x = (self.position.root.x - 1) * @intFromBool(self.position.root.x > 1);
    }
    if (cur_gi.y < 0) {
        self.position.offset.y = 0;
        self.position.root.y = (self.position.root.y - 1) * @intFromBool(self.position.root.y > 1);
    }
    if (cur_gi.x >= imax.x - 1) {
        self.position.offset.x = @as(f32, @floatFromInt(imax.x - 2)) * self.grid_item.x - pad;
        self.position.root.x += 1;
    }
    if (cur_gi.y >= imax.y - 1) {
        self.position.offset.y = @as(f32, @floatFromInt(imax.y - 2)) * self.grid_item.y - pad;
        self.position.root.y += 1;
    }
}

pub fn iter(self: VSelf) IterVisible {
    const gmax = self.max();
    return .{
        .view = self,
        .cur = 0,
        .width = gmax.x,
        .total = gmax.x * gmax.y,
    };
}

const IterVisible = struct {
    view: VSelf,
    cur: isize,
    width: isize,
    total: isize,

    pub fn next(self: *@This()) ?struct {
        ix: C(isize),
        rect: Rect(f32),
    } {
        if (self.cur == self.total) return null;
        defer self.cur += 1;
        const grid_ix = .{
            .x = @mod(self.cur, self.width),
            .y = @divFloor(self.cur, self.width),
        };

        const grid_ix_pos = C(f32)
            .binopChain(.mul, grid_ix, self.view.grid_item)
            .finally(.add, self.view.window);

        return .{ .ix = grid_ix, .rect = .{
            .x = grid_ix_pos.x,
            .y = grid_ix_pos.y,
            .width = self.view.grid_item.x,
            .height = self.view.grid_item.y,
        } };
    }
};

pub fn format(value: VSelf, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
    try writer.print("GridView {{ position: {}, window {}, grid_specs {}}", .{
        value.position,
        value.window,
        C(isize).binop(.div, value.window.width, value.grid_item.x),
    });
}
