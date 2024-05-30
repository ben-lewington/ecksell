const std = @import("std");
pub fn IndexStr(comptime alphabet: []const u8) type {
    const base = alphabet.len;
    const max_len = freeMonoidReprLen(base, std.math.maxInt(isize));
    comptime {
        for (0..base) |i|
            for (i + 1..base) |j|
                if (alphabet[i] == alphabet[j])
                    @compileError("alphabet must be unique");
    }

    return struct {
        fn castInput(index: anytype) usize {
            const Ty = @TypeOf(index);
            if (Ty == usize) return index;
            if (Ty == isize and index < 0) return 0;
            if (Ty == isize) return @as(usize, index);
            if (Ty == comptime_int and index < 0) return 0;
            if (Ty == comptime_int) return @as(usize, index);
            @compileError(std.fmt.comptimePrint("unsupported type {s}", .{@typeName(Ty)}));
        }

        pub fn convert(index: anytype) struct { buf: [max_len]u8, len: usize } {
            var val: usize = castInput(index);
            const len = freeMonoidReprLen(base, index);

            var ret: [max_len]u8 = undefined;
            for (0..len) |i| {
                ret[len - 1 - i] = alphabet[val % base];
                if (val / base >= 1) {
                    val = (val / base) - 1;
                } else break;
            }

            return .{
                .buf = ret,
                .len = len,
            };
        }
    };
}

fn freeMonoidReprLen(base: usize, p: usize) usize {
    var pp = p;
    var ret: usize = 1;

    while (pp != 0) {
        // if this overflows, as p is a usize, it must be of len `ret`
        const br = std.math.powi(usize, base, ret) catch return ret;

        if (pp < br) return ret;

        pp -= br;
        ret += 1;
    }

    return ret;
}
