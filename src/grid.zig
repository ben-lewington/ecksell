const std = @import("std");

pub const rl = @cImport({
    @cInclude("raylib.h");
    @cDefine("RAYGUI_IMPLEMENTATION", {});
    @cInclude("raygui.h");
});
const Self = @This();

virt_root: Coord(isize),
virt_offset: Coord(f32),

pub fn Coord(comptime T: type) type {
    if (!(T == isize or T == f32 or T == usize))
        @compileError(std.fmt.comptimePrint("expected numeric type for {}", @typeName(@This())));
    return struct {
        x: T,
        y: T,
        const CSelf = @This();

        pub fn @"0"() CSelf {
            return .{ .x = 0, .y = 0 };
        }

        pub fn toVec2(self: CSelf) rl.Vector2 {
            return .{ .x = self.x, .y = self.y };
        }

        pub fn format(value: CSelf, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
            try writer.print("{d}, {d}", .{ value.x, value.y });
        }

        const Ops = enum(u8) {
            add = '+',
            sub = '-',
            mul = '*',
            div = '/',

            inline fn do(self: @This(), l: anytype, r: anytype) T {
                return switch (self) {
                    .add => l + r,
                    .sub => l - r,
                    .mul => l * r,
                    .div => l / r,
                };
            }

            inline fn ido(self: @This(), l: anytype, r: anytype) void {
                return switch (self) {
                    .add => l.* += r,
                    .sub => l.* -= r,
                    .mul => l.* *= r,
                    .div => l.* /= r,
                };
            }
        };

        pub const BinOpChain = struct {
            inner: CSelf,

            pub fn then(self: *@This(), op: Ops, rhs: anytype) *@This() {
                op.ido(&self.inner.x, rhs);
                op.ido(&self.inner.y, rhs);
                return self;
            }

            pub fn finally(self: *@This(), op: Ops, rhs: anytype) CSelf {
                op.ido(&self.inner.x, rhs);
                op.ido(&self.inner.y, rhs);
                return self.inner;
            }
        };

        pub inline fn binopChain(comptime op: Ops, lhs: anytype, rhs: anytype) BinOpChain {
            return .{ .inner = .{
                .x = op.do(castCoord(lhs.x), castCoord(rhs.x)),
                .y = op.do(castCoord(lhs.y), castCoord(rhs.y)),
            } };
        }

        pub inline fn binop(comptime op: Ops, lhs: anytype, rhs: anytype) @This() {
            return .{
                .x = op.do(castCoord(lhs.x), castCoord(rhs.x)),
                .y = op.do(castCoord(lhs.y), castCoord(rhs.y)),
            };
        }

        pub inline fn ibinop(self: *@This(), comptime op: Ops, rhs: anytype) void {
            op.ido(&self.x, rhs.x);
            op.ido(&self.y, rhs.y);
        }

        pub inline fn ibinopPipe(self: *@This(), comptime op: Ops, rhs: anytype) *@This() {
            self.ibinop(op, rhs);
            return self;
        }

        pub inline fn shear(n: anytype) @This() {
            const nc = castCoord(n);
            return .{ .x = nc, .y = nc };
        }

        pub inline fn ishear(self: *@This(), n: anytype) *@This() {
            self.ibinop(.add, @This().shear(castCoord(n)));
            return self;
        }

        inline fn checkValidPoint(x: anytype) ?type {
            const Ty = @TypeOf(x);
            if (!(Ty == isize or Ty == f32 or Ty == usize or Ty == comptime_int)) return null;
            return Ty;
        }

        inline fn castCoord(foo: anytype) T {
            if (checkValidPoint(foo)) |Ty| {
                if (T == Ty) return foo;
                if (Ty == f32 and (T == isize or T == usize or T == comptime_int)) return @intFromFloat(foo);
                if ((Ty == isize or Ty == usize or Ty == comptime_int) and T == f32) return @floatFromInt(foo);
                if ((Ty == isize or Ty == usize or Ty == comptime_int) and (T == isize or T == usize or T == comptime_int)) return @intCast(foo);
                @compileError(std.fmt.comptimePrint("unhandled coord cast: T: {s} and typeof(foo): {s}", .{ @typeName(T), @typeName(@TypeOf(foo)) }));
            } else @compileError(std.fmt.comptimePrint("{s} is not a valid grid point, must be a numeric type", .{@typeName(@TypeOf(foo))}));
        }
    };
}

pub fn Rect(comptime T: type) type {
    return struct {
        x: T,
        y: T,
        width: T,
        height: T,

        pub fn as(self: @This()) rl.Rectangle {
            return .{
                .x = self.x,
                .y = self.y,
                .width = self.width,
                .height = self.height,
            };
        }

        pub fn format(value: @This(), comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
            try writer.print("{d}x{d}->({d}, {d})", .{ value.x, value.y, value.width, value.width });
        }
    };
}

pub const View = struct {
    inner: Self,
    window: rl.Rectangle,
    grid_item: Coord(f32),

    const VSelf = @This();

    pub fn currentCell(self: *const VSelf) rl.Rectangle {
        const lattice = .{
            .x = 1.5 * self.grid_item.x + self.inner.virt_offset.x,
            .y = 1.5 * self.grid_item.y + self.inner.virt_offset.y,
        };

        return .{
            .x = self.window.x + self.grid_item.x * std.math.floor(lattice.x / self.grid_item.x),
            .y = self.window.y + self.grid_item.y * std.math.floor(lattice.y / self.grid_item.y),
            .width = self.grid_item.x,
            .height = self.grid_item.y,
        };
    }

    pub fn max(self: *const VSelf) Coord(isize) {
        return .{
            .x = @intFromFloat(@divFloor(self.window.width, self.grid_item.x)),
            .y = @intFromFloat(@divFloor(self.window.height, self.grid_item.y)),
        };
    }

    pub fn moveCursor(self: *VSelf, offset: Coord(f32), pad: f32) void {
        std.log.debug("offset {}", .{offset});
        const imax = self.max();
        if (offset.x == 0 and offset.y == 0) return;

        self.inner.virt_offset.ibinop(.add, offset);

        const cur_gi = self.currentCursorIndex();
        if (cur_gi.x < 0) {
            self.inner.virt_offset.x = 0;
            self.inner.virt_root.x = (self.inner.virt_root.x - 1) * @intFromBool(self.inner.virt_root.x > 1);
        }
        if (cur_gi.y < 0) {
            self.inner.virt_offset.y = 0;
            self.inner.virt_root.y = (self.inner.virt_root.y - 1) * @intFromBool(self.inner.virt_root.y > 1);
        }
        if (cur_gi.x >= imax.x - 1) {
            self.inner.virt_offset.x = @as(f32, @floatFromInt(imax.x - 2)) * self.grid_item.x - pad;
            self.inner.virt_root.x += 1;
        }
        if (cur_gi.y >= imax.y - 1) {
            self.inner.virt_offset.y = @as(f32, @floatFromInt(imax.y - 2)) * self.grid_item.y - pad;
            self.inner.virt_root.y += 1;
        }
    }

    pub fn currentCursorIndex(self: *const VSelf) Coord(isize) {
        const lattice: Coord(f32) = .{
            .x = 1.5 * self.grid_item.x + self.inner.virt_offset.x,
            .y = 1.5 * self.grid_item.y + self.inner.virt_offset.y,
        };
        return .{
            .x = @as(isize, @intFromFloat(@floor(lattice.x / self.grid_item.x))) - 1,
            .y = @as(isize, @intFromFloat(@floor(lattice.y / self.grid_item.y))) - 1,
        };
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
            ix: Coord(isize),
            rect: rl.Rectangle,
        } {
            if (self.cur == self.total) return null;
            defer self.cur += 1;
            const grid_ix = .{
                .x = @mod(self.cur, self.width),
                .y = @divFloor(self.cur, self.width),
            };
            return .{ .ix = grid_ix, .rect = .{
                .x = self.view.window.x + (@as(f32, @floatFromInt(grid_ix.x)) * self.view.grid_item.x),
                .y = self.view.window.y + (@as(f32, @floatFromInt(grid_ix.y)) * self.view.grid_item.y),
                .width = self.view.grid_item.x,
                .height = self.view.grid_item.y,
            } };
        }
    };

    pub fn format(value: VSelf, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("GridView {{ window: {d}x{d}->({d}, {d}), grid_dims: {d}x{d}, grid_specs ({d}, {d}) }}", .{
            value.window.width,
            value.window.height,
            value.window.x,
            value.window.y,
            value.grid_item.x,
            value.grid_item.y,
            @divFloor(value.window.width, value.grid_item.x),
            @divFloor(value.window.height, value.grid_item.y),
        });
    }
};
