const std = @import("std");
const Coord = @import("coord.zig").Coord;
const rl = @import("../raylib.zig").include();

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

pub fn Range(comptime T: type) type {
    return struct {
        top_left: Coord(T),
        bottom_right: Coord(T),

        pub inline fn contains(self: *const @This(), coord: Coord(T)) bool {
            return (coord.x >= self.top_left.x and coord.y >= self.top_left.y and
                coord.x <= self.bottom_right.x and coord.y <= self.bottom_right.y);
        }

        // TODO(BL): finish this!, will need it soon
        pub fn intersection(self: *const @This(), other: @This()) ?@This() {
            if (self.top_left.x > other.bottom_right.x or self.top_left.y > other.bottom_right.y or
                other.top_left.x > self.bottom_right.x or other.top_left.y > self.bottom_right.y) return null;
            if ((self.top_left.x == other.bottom_right.x and self.top_left.y == other.bottom_right.y) or
                (self.top_left.x == other.bottom_right.x and self.top_left.y == other.bottom_right.y)) return null;

            return null;
        }
    };
}
