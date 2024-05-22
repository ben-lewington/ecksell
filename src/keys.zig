const std = @import("std");
pub const rl = @cImport({
    @cInclude("raylib.h");
    @cDefine("RAYGUI_IMPLEMENTATION", {});
    @cInclude("raygui.h");
});

pub const KeyboardKey = enum(c_int) {
    null = 0,
    apostrophe = 39,
    comma = 44,
    minus = 45,
    period = 46,
    slash = 47,
    zero = 48,
    one = 49,
    two = 50,
    three = 51,
    four = 52,
    five = 53,
    six = 54,
    seven = 55,
    eight = 56,
    nine = 57,
    semicolon = 59,
    equal = 61,
    a = 65,
    b = 66,
    c = 67,
    d = 68,
    e = 69,
    f = 70,
    g = 71,
    h = 72,
    i = 73,
    j = 74,
    k = 75,
    l = 76,
    m = 77,
    n = 78,
    o = 79,
    p = 80,
    q = 81,
    r = 82,
    s = 83,
    t = 84,
    u = 85,
    v = 86,
    w = 87,
    x = 88,
    y = 89,
    z = 90,
    left_bracket = 91,
    backslash = 92,
    right_bracket = 93,
    grave = 96,
    space = 32,
    escape = 256,
    enter = 257,
    tab = 258,
    backspace = 259,
    insert = 260,
    delete = 261,
    right = 262,
    left = 263,
    down = 264,
    up = 265,
    page_up = 266,
    page_down = 267,
    home = 268,
    end = 269,
    caps_lock = 280,
    scroll_lock = 281,
    num_lock = 282,
    print_screen = 283,
    pause = 284,
    f1 = 290,
    f2 = 291,
    f3 = 292,
    f4 = 293,
    f5 = 294,
    f6 = 295,
    f7 = 296,
    f8 = 297,
    f9 = 298,
    f10 = 299,
    f11 = 300,
    f12 = 301,
    left_shift = 340,
    left_control = 341,
    left_alt = 342,
    left_super = 343,
    right_shift = 344,
    right_control = 345,
    right_alt = 346,
    right_super = 347,
    kb_menu = 348,
    kp_0 = 320,
    kp_1 = 321,
    kp_2 = 322,
    kp_3 = 323,
    kp_4 = 324,
    kp_5 = 325,
    kp_6 = 326,
    kp_7 = 327,
    kp_8 = 328,
    kp_9 = 329,
    kp_decimal = 330,
    kp_divide = 331,
    kp_multiply = 332,
    kp_subtract = 333,
    kp_add = 334,
    kp_enter = 335,
    kp_equal = 336,
    back = 4,
    menu = 5,
    volume_up = 24,
    volume_down = 25,

    const Self = @This();

    pub inline fn literally(s: Self) c_int {
        return @intFromEnum(s);
    }

    pub fn CaptureGroup(comptime S: type, comptime unique_keys: []const Self) type {
        const keys = @typeInfo(Self).Enum.fields;
        comptime {
            switch (@typeInfo(S)) {
                .Enum => |ef| {
                    if (ef.fields.len != unique_keys.len) @compileError("provided enum does not match the length of the keys");
                },
                else => {
                    const loc = @src();
                    @compileError(std.fmt.comptimePrint("{s}:{d}:{d}: in `{s}` expected enum.", .{
                        loc.file,
                        loc.line,
                        loc.column,
                        loc.fn_name,
                    }));
                },
            }
            if (unique_keys.len > keys.len) @compileError("must be a unique set of keys");
            for (unique_keys, 0..) |uq, i| {
                for (i + 1..unique_keys.len) |j| {
                    if (uq == unique_keys[j]) @compileError("must be a unique set of keys");
                }
            }
        }
        return struct {
            inner: u128,

            pub fn getFrameKeys() @This() {
                var ret: u128 = 0;
                inline for (unique_keys, 0..) |uq, key_ix| {
                    if (rl.IsKeyPressed(uq.literally()) or rl.IsKeyDown(uq.literally())) {
                        ret |= (1 << key_ix);
                    }
                }
                return .{ .inner = ret };
            }

            pub inline fn isKeySet(self: *const @This(), s: S) bool {
                return (self.inner & (1 << @intFromEnum(s))) > 0;
            }

            pub fn format(value: @This(), comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
                try writer.print("CaptureGroup {{", .{});
                inline for (@typeInfo(S).Enum.fields, 0..) |f, i| {
                    try writer.print("{s}: {}, ", .{
                        f.name,
                        value.inner & (1 << i) > 0,
                    });
                }
                try writer.print("}}", .{});
            }
        };
    }
};
