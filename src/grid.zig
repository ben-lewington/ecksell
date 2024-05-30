const std = @import("std");
pub const Coord = @import("grid/coord.zig").Coord;
pub const area = @import("grid/2d.zig");
pub const Render = @import("grid/Render.zig");
pub const AlphabetIx = @import("grid/alphabet_indexing.zig").IndexStr;

pub fn Cell(comptime C: type) type {
    return struct {
        at: Coord(isize),
        contents: C,
        pub fn format(value: @This(), comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
            try writer.print("@({})->{}", .{ value.at, value.contents });
        }
    };
}
