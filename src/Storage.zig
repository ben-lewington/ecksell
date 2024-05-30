const std = @import("std");
const rl = @import("raylib.zig").include();
const grid = @import("grid.zig");

const str = []const u8;
const Arr = std.ArrayList;
const Cell = grid.Cell(Field.Value);
const GridIx = grid.Coord(isize);

store: Heap,
boundary: struct {
    top_left: GridIx,
    bottom_right: GridIx,

    pub inline fn dimensions(self: @This()) GridIx {
        return GridIx
            .binop(.sub, self.bottom_right, self.top_left);
    }
},
regions: Arr(Heap.Region.Value),

const SSelf = @This();

pub inline fn hasData(self: *const @This()) bool {
    return self.regions.items.len > 0;
}

pub fn getValueAt(self: *const @This(), ix: GridIx) ?Field.Value {
    for (self.regions.items) |region| {
        switch (region) {
            .cell => |c_ix| {
                const cell = self.store.cells.items[c_ix.index];
                if (GridIx.equ(ix, cell.at)) return cell.contents;
            },
            .cluster => |cell_slice| {
                const cells = self.store.cells.items[cell_slice.start..cell_slice.end];
                for (cells) |c| if (GridIx.equ(ix, c.at)) return c.contents;
            },
            else => std.debug.assert(false),
            // .tabular,
        }
    }
    return null;
}

pub fn storeValueAt(self: *@This(), ix: GridIx, value: []u8) ?Cell {
    var nbors: [8]Cell = undefined;
    var nlen: usize = 0;

    for (self.regions.items) |region| {
        switch (region) {
            .cell => |c_ix| {
                const cell = self.store.cells.items[c_ix.index];
                const delta = GridIx.binop(.sub, cell.at, ix);

                if (GridIx.equ(delta, GridIx.@"0"())) {
                    // std.debug.assert(false);
                    self.store.cells.items[c_ix.index].contents = self.parseValue(value);
                    return cell;
                } else if (@abs(delta.x) <= 1 and @abs(delta.y) <= 1) {
                    for (nbors[0..nlen]) |n| {
                        if (GridIx.equ(n.at, cell.at)) {
                            nbors[nlen] = cell;
                            nlen += 1;
                            break;
                        }
                    }
                }
            },
            .cluster => |cell_slice| {
                const cells = self.store.cells.items[cell_slice.start..cell_slice.end];
                for (cells, 0..) |c, i| {
                    const delta = GridIx.binop(.sub, c.at, ix);

                    if (GridIx.equ(delta, GridIx.@"0"())) {
                        // std.debug.assert(false);
                        self.store.cells.items[cell_slice.start + i].contents = self.parseValue(value);
                        return c;
                    } else if (@abs(delta.x) <= 1 and @abs(delta.y) <= 1) {
                        for (nbors[0..nlen]) |n| {
                            if (GridIx.equ(n.at, c.at)) {
                                nbors[nlen] = c;
                                nlen += 1;
                                break;
                            }
                        }
                    }
                }
            },
            else => std.debug.assert(false),
        }
    }

    var val: Heap.Region.Value = undefined;
    const cell_region_start = self.store.cells.items.len;
    const cell = Cell{ .at = ix, .contents = self.parseValue(value) };

    if (nlen > 0) {
        for (nbors[0..nlen]) |n| {
            self.store.cells.append(n) catch unreachable;
        }
        self.store.cells.append(cell) catch unreachable;
        val = .{
            .cluster = .{
                .start = cell_region_start,
                .end = self.store.cells.items.len,
            },
        };
    } else {
        self.store.cells.append(cell) catch unreachable;
        std.log.info("stored {}", .{cell});
        val = .{ .cell = .{ .index = cell_region_start } };
    }
    self.regions.append(val) catch unreachable;
    return null;
}

pub fn parseValue(self: *SSelf, input: []const u8) Field.Value {
    const i = std.fmt.parseInt(isize, input, 10) catch {
        const f = std.fmt.parseFloat(f32, input) catch {
            const id = self.store.text.items.len;
            self.store.text.appendSlice(input) catch unreachable;
            return .{ .text = .{ .start = id, .end = self.store.text.items.len } };
        };
        return .{ .real = f };
    };
    return .{ .integer = i };
}

pub const Heap = struct {
    cells: Arr(Cell),
    text: Arr(u8),
    indices: Arr(usize),
    integers: Arr(isize),
    reals: Arr(f32),

    pub fn create() @This() {
        const alc = std.heap.ArenaAllocator.init(std.heap.page_allocator);
        _ = alc;
    }

    pub fn format(value: @This(), comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("Heap {{ cells: [{s}] }}", .{value.cells.items});
    }

    /// a ptr that can be associated with a dynamic buffer that will remain valid after relocating the buffer
    fn RelPtr(comptime T: type) type {
        return struct {
            index: usize,
            pub fn Repr() type {
                return T;
            }
        };
    }

    /// a slice that can be associated with a dynamic buffer that will remain valid after relocating the buffer
    fn RelSlice(comptime T: type) type {
        return struct {
            start: usize,
            end: usize,

            pub fn Repr() type {
                return T;
            }
        };
    }

    pub const Region = enum {
        cell,
        cluster,
        tabular,

        pub const Value = union(Region) {
            cell: RelPtr(Cell),
            cluster: RelSlice(Cell),
            tabular: ColumnarTable,
        };

        pub const ColumnarTable = struct {
            top_left: GridIx,
            row_len: isize,
            columns: Arr(Col),

            pub const Col = struct {
                name: str,
                data: Field.Slice,
            };
        };
    };
};

pub fn guiTextFormat(self: SSelf, v: Field.Value) [*c]const u8 {
    return switch (v) {
        .integer => |num| rl.TextFormat("%d", num),
        .real => |num| rl.TextFormat("%f", num),
        .text => |t| rl.TextFormat("%.*s", t.end - t.start, self.store.text.items[t.start..t.end].ptr),
    };
}

pub const Field = enum {
    integer,
    real,
    text,

    const n = @typeInfo(Field).Enum.fields.len;

    const RelSlice = Heap.RelSlice;

    pub const pallette: [n]rl.Color = [_]rl.Color{
        rl.BLUE,
        rl.GREEN,
        rl.PINK,
    };

    pub const Value = union(Field) {
        integer: isize,
        real: f32,
        text: RelSlice(u8),

        pub fn format(value: Value, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
            switch (value) {
                .integer => |i| try writer.print("{d}", .{i}),
                .real => |i| try writer.print("{d}", .{i}),
                .text => |t| try writer.print("{d}->{d}", .{ t.start, t.end }),
            }
            try writer.print("[{s}]", .{@tagName(value)});
        }
    };

    pub const Slice = union(Field) {
        integer: RelSlice(isize),
        real: RelSlice(f32),
        text: struct {
            raw: RelSlice(u8),
            offsets: RelSlice(usize),
        },
    };
};
