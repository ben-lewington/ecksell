const std = @import("std");

pub fn Coord(comptime T: type) type {
    if (!(T == isize or T == f32 or T == usize))
        @compileError(std.fmt.comptimePrint("expected numeric type for {s}", .{@typeName(@This())}));
    return struct {
        x: T,
        y: T,
        const CSelf = @This();

        pub fn @"0"() CSelf {
            return .{ .x = 0, .y = 0 };
        }

        pub fn new(x: anytype, y: anytype) @This() {
            return .{ .x = castCoordPoint(x), .y = castCoordPoint(y) };
        }

        pub fn castCoord(self: CSelf, comptime S: type) Coord(S) {
            return .{ .x = castCoordPoint(self.x), .y = castCoordPoint(self.y) };
        }

        pub fn equ(lhs: anytype, rhs: anytype) bool {
            return castCoordPoint(lhs.x) == castCoordPoint(rhs.x) and
                castCoordPoint(lhs.y) == castCoordPoint(rhs.y);
        }

        pub fn manhattanLen(coord: anytype) usize {
            return @abs(castCoordPoint(coord.x)) + @abs(castCoordPoint(coord.y));
        }

        pub fn euclideanLen(coord: anytype) usize {
            return std.math.pow(castCoordPoint(coord.x), 2) + std.math.pow(castCoordPoint(coord.y), 2);
        }

        pub fn dist(comptime len: fn (anytype) usize, lhs: anytype, rhs: anytype) usize {
            (len)(binop(.sub, lhs, rhs));
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
                    .div => if (T != f32) @divFloor(l, r) else l / r,
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

        pub inline fn binop(op: Ops, lhs: anytype, rhs: anytype) @This() {
            return .{
                .x = op.do(castCoordPoint(lhs.x), castCoordPoint(rhs.x)),
                .y = op.do(castCoordPoint(lhs.y), castCoordPoint(rhs.y)),
            };
        }

        pub inline fn binopChain(op: Ops, lhs: anytype, rhs: anytype) BinOpChain {
            return .{ .inner = .{
                .x = op.do(castCoordPoint(lhs.x), castCoordPoint(rhs.x)),
                .y = op.do(castCoordPoint(lhs.y), castCoordPoint(rhs.y)),
            } };
        }

        pub inline fn ibinop(self: *@This(), comptime op: Ops, rhs: anytype) void {
            op.ido(&self.x, castCoordPoint(rhs.x));
            op.ido(&self.y, castCoordPoint(rhs.y));
        }

        pub inline fn chain(self: @This()) BinOpChain {
            return .{ .inner = self };
        }

        pub const BinOpChain = struct {
            inner: CSelf,

            pub fn then(self: @This(), op: Ops, rhs: anytype) @This() {
                return .{ .inner = .{
                    .x = op.do(self.inner.x, castCoordPoint(rhs.x)),
                    .y = op.do(self.inner.y, castCoordPoint(rhs.y)),
                } };
            }

            pub fn finally(self: @This(), op: Ops, rhs: anytype) CSelf {
                return .{
                    .x = op.do(self.inner.x, castCoordPoint(rhs.x)),
                    .y = op.do(self.inner.y, castCoordPoint(rhs.y)),
                };
            }
        };

        pub inline fn shear(n: anytype) @This() {
            const nc = castCoordPoint(n);
            return .{ .x = nc, .y = nc };
        }

        inline fn checkValidPoint(x: anytype) ?type {
            const Ty = @TypeOf(x);
            if (!(Ty == isize or Ty == f32 or Ty == usize or Ty == comptime_int or Ty == comptime_float)) return null;
            return Ty;
        }

        inline fn castCoordPoint(foo: anytype) T {
            if (checkValidPoint(foo)) |Ty| {
                if (T == Ty) return foo;
                if (T == f32 and Ty == comptime_float) return foo;
                if ((Ty == f32 or Ty == comptime_float) and (T == isize or T == usize or T == comptime_int)) return @intFromFloat(foo);
                if ((Ty == isize or Ty == usize or Ty == comptime_int) and (T == f32 or T == comptime_float)) return @floatFromInt(foo);
                if ((Ty == isize or Ty == usize or Ty == comptime_int) and (T == isize or T == usize or T == comptime_int)) return @intCast(foo);
                @compileError(std.fmt.comptimePrint("unhandled coord cast: T: {s} and typeof(foo): {s}", .{ @typeName(T), @typeName(@TypeOf(foo)) }));
            } else @compileError(std.fmt.comptimePrint("{s} is not a valid grid point, must be a numeric type", .{@typeName(@TypeOf(foo))}));
        }

        pub fn format(value: CSelf, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
            try writer.print("{d}, {d}", .{ value.x, value.y });
        }
    };
}
