const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});

    const optimize = b.standardOptimizeOption(.{});

    if (target.result.os.tag == .macos) {
        std.debug.print("Target operating system is not supported by EE3D.\n", .{});
    }

    const exe_mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });

    const zglfw = b.dependency("zglfw", .{});
    exe_mod.addImport("zglfw", zglfw.module("root"));

    if (target.result.os.tag != .emscripten) {
        exe_mod.linkLibrary(zglfw.artifact("glfw"));
    }

    const zopengl = b.dependency("zopengl", .{});
    exe_mod.addImport("zopengl", zopengl.module("root"));

    const zigstbi = b.dependency("ZigSTBI", .{});
    exe_mod.addImport("zigstbi", zigstbi.module("zigstbi"));

    const nfd = b.dependency("NFD", .{});
    exe_mod.addImport("nfd", nfd.module("nfd"));

    const ziglibraw = b.dependency("ZigLibRaw", .{});

    const zgui = b.dependency("zgui", .{
        .shared = false,
        .with_implot = false,
        .backend = .glfw_opengl3,
        .target = target,
    });
    exe_mod.addImport("zgui", zgui.module("root"));

    exe_mod.linkLibrary(zgui.artifact("imgui"));
    exe_mod.linkLibrary(ziglibraw.artifact("raw"));
    exe_mod.addIncludePath(ziglibraw.path("libraw"));

    const exe = b.addExecutable(.{
        .name = "EvilStackr",
        .root_module = exe_mod,
    });
    b.installArtifact(exe);
    b.installBinFile("imgui.ini", "imgui.ini");
    b.installDirectory(.{ .install_dir = .bin, .source_dir = b.path("assets/"), .install_subdir = "assets/" });

    const run_cmd = b.addRunArtifact(exe);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    const exe_unit_tests = b.addTest(.{
        .root_module = exe_mod,
    });

    const run_exe_unit_tests = b.addRunArtifact(exe_unit_tests);

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_exe_unit_tests.step);
}
