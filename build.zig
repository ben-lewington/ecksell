const raylib = @import("raylib");
const std = @import("std");
// const CreateOptions = std.Build.CreateOptions;

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    // const mod = b.addModule("ecksell", .{
    //     .root_source_file = .{ .path = "src/lib.zig" },
    // });

    // _ = mod;

    // mod.
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});

    const rl = try raylib.addRaylib(b, target, optimize, .{});
    b.installArtifact(rl);

    // const rg = b.addStaticLibrary(.{
    //     .name = "raygui",
    //     .target = target,
    //     .optimize = optimize,
    // });
    // rg.addIncludePath(rl.getEmittedIncludeTree());
    // rg.linkLibC();
    // rg.defineCMacro("RAYGUI_IMPLEMENTATION", null);
    // rg.addCSourceFile(.{ .file = .{ .path = "lib/raygui/src/raygui.h" } });

    // b.installArtifact(rg);

    const exe = b.addExecutable(.{
        .name = "zigfoo",
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    exe.addIncludePath(rl.getEmittedIncludeTree());
    exe.addIncludePath(.{ .path = "lib/raygui/src" });
    exe.addRPath(.{ .path = "src" });
    exe.linkLibrary(rl);
    // exe.linkLibrary(rg);
    // exe.defineCMacro("", value: ?[]const u8)
    // exe.addCSourceFile(.{ .file = .{ .path = "foo.c" } });

    b.installArtifact(exe);

    const run = b.addRunArtifact(exe);

    run.step.dependOn(b.getInstallStep());

    // This allows the user to pass arguments to the application in the build
    // command itself, like this: `zig build run -- arg1 arg2 etc`
    if (b.args) |args| {
        run.addArgs(args);
    }

    const run_step = b.step("run", "run the app");
    run_step.dependOn(&run.step);
}
