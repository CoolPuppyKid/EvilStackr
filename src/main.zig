const std = @import("std");
const zgui = @import("zgui");
const glfw = @import("zglfw");
const zopengl = @import("zopengl");
const zigstbi = @import("zigstbi");
const c = @import("c.zig").c;
const UI = @import("UI/ui.zig").UI;
const Assets = @import("assets.zig");
const Window = @import("window.zig").Window;
const nfd = @import("nfd");
const allocator = std.heap.page_allocator;
const gl = zopengl.bindings;

pub fn main() !void {
    const processor = c.libraw_init(0) orelse return error.FailedToInitLibRaw;
    defer c.libraw_close(processor);
    std.debug.print("Initialized LibRaw: {s}\n", .{c.libraw_version()});

    try glfw.init();
    defer glfw.terminate();
    glfw.windowHint(.context_version_major, 4);
    glfw.windowHint(.context_version_minor, 0);
    glfw.windowHint(.opengl_profile, .opengl_core_profile);
    glfw.windowHint(.opengl_forward_compat, true);
    glfw.windowHint(.client_api, .opengl_api);
    glfw.windowHint(.doublebuffer, true);

    var window = try Window.init(allocator);
    defer window.deinit();

    try window.setIcon("EvilStackr.png");

    var ui = UI.init(allocator, window);
    defer ui.deinit();
    ui.setDarkTheme();

    const testPath = try Assets.getPathOfFileAlloc("test.png", allocator);
    defer allocator.free(testPath);

    while (!window.shouldClose()) {
        glfw.pollEvents();

        gl.clearBufferfv(gl.COLOR, 0, &[_]f32{ 0, 0, 0, 1.0 });

        ui.startRender();
        _ = zgui.begin("Viewport", .{});
        zgui.end();

        _ = zgui.begin("Project", .{});
        zgui.end();
        ui.endRender();

        window.rawWindow.swapBuffers();
    }
}
