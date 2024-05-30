const raylib = @import("raylib");
const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});

    const optimize = b.standardOptimizeOption(.{});

    const rl = try raylib.addRaylib(b, target, optimize, .{});
    b.installArtifact(rl);

    const exe = b.addExecutable(.{
        .name = "zl_sheets",
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    exe.addIncludePath(rl.getEmittedIncludeTree());
    exe.addIncludePath(.{ .path = "lib/raygui/src" });
    exe.addRPath(.{ .path = "src" });
    exe.linkLibrary(rl);

    b.installArtifact(exe);

    const run = b.addRunArtifact(exe);

    run.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run.addArgs(args);
    }

    const run_step = b.step("run", "run the app");
    run_step.dependOn(&run.step);
}
