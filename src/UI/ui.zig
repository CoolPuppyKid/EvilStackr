const std = @import("std");
const zgui = @import("zgui");
const glfw = @import("zglfw");
const zopengl = @import("zopengl");
const colors = @import("../colors.zig");
const Window = @import("../window.zig").Window;

pub const UI = struct {
    allocator: std.mem.Allocator,
    window: Window,

    pub fn init(allocator: std.mem.Allocator, window: Window) UI {
        zgui.init(allocator);
        zgui.backend.init(window.rawWindow);
        zgui.io.setConfigFlags(.{ .dock_enable = true, .viewport_enable = true });
        const scale_factor = scale_factor: {
            const scale = window.rawWindow.getContentScale();
            break :scale_factor @max(scale[0], scale[1]);
        };

        zgui.getStyle().scaleAllSizes(scale_factor);

        return UI{ .allocator = allocator, .window = window };
    }

    pub fn setDarkTheme(self: *UI) void {
        _ = self;
        const style = zgui.getStyle();
        styleSetColor(style, .text, 255, 255, 255, 255);
        styleSetColor(style, .window_bg, 23, 14, 26, 255);
        styleSetColor(style, .title_bg_active, 43, 82, 136, 255);
        styleSetColor(style, .title_bg_collapsed, 39, 50, 66, 255);
        styleSetColor(style, .title_bg, 39, 50, 66, 255);
        _ = zgui.io.addFontFromFile("assets/Fonts/M_PLUS_Rounded_1c/MPLUSRounded1c-ExtraBold.ttf", 18);
        style.window_rounding = 0.2;
    }

    fn styleSetColor(Style: *zgui.Style, Col: zgui.StyleCol, R: u32, G: u32, B: u32, A: u32) void {
        Style.colors[@as(usize, @intCast(@intFromEnum(Col)))] = colors.rgba255to1f(.{ .r = R, .g = G, .b = B, .a = A });
    }

    pub fn startRender(self: *UI) void {
        const fb_size = self.window.rawWindow.getFramebufferSize();

        zgui.backend.newFrame(@intCast(fb_size[0]), @intCast(fb_size[1]));
        _ = zgui.dockSpaceOverViewport(0, zgui.getMainViewport(), .{ .passthru_central_node = true });
    }

    pub fn endRender(self: *UI) void {
        zgui.backend.draw();
        zgui.updatePlatformWindows();
        zgui.renderPlatformWindowsDefault();
        glfw.makeContextCurrent(self.window.rawWindow);
    }

    pub fn deinit(self: *UI) void {
        _ = self;
        zgui.backend.deinit();
        zgui.deinit();
    }
};
