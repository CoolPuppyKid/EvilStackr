const std = @import("std");
const zgui = @import("zgui");
const glfw = @import("zglfw");
const zopengl = @import("zopengl");
const zigstbi = @import("zigstbi");
const Assets = @import("assets.zig");

pub const Window = struct {
    pub fn init(allocator: std.mem.Allocator) !Window {
        const window = try glfw.Window.create(800, 500, "EvilStackr", null);

        glfw.makeContextCurrent(window);
        glfw.swapInterval(1);

        try zopengl.loadCoreProfile(glfw.getProcAddress, 4, 0);

        return Window{ .rawWindow = window, .allocator = allocator };
    }

    pub fn setIcon(self: *Window, icon: []const u8) !void {
        const iconPath = try std.fs.path.join(self.allocator, &[_][]const u8{ "UI", icon });
        defer self.allocator.free(iconPath);

        const assetIconPath = try Assets.getPathOfFileAlloc(iconPath, self.allocator);
        defer self.allocator.free(assetIconPath);

        var images: [1]glfw.Image = undefined;
        var stbiImg = try zigstbi.load_file(assetIconPath, 0);
        defer stbiImg.deinit();

        images[0] = .{
            .width = @intCast(stbiImg.width),
            .height = @intCast(stbiImg.height),
            .pixels = stbiImg.bytes.ptr,
        };
        glfw.setWindowIcon(self.rawWindow, &images);
    }

    pub fn shouldClose(self: *Window) bool {
        return self.rawWindow.shouldClose();
    }

    pub fn deinit(self: *Window) void {
        self.rawWindow.destroy();
    }
    rawWindow: *glfw.Window,
    allocator: std.mem.Allocator,
};
