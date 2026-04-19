const std = @import("std");

/// RGB but 1 is brightest value and 0 is the darkest
pub const rgba1 = struct {
    r: f32,
    g: f32,
    b: f32,
    a: f32,
};

/// RGB but 255 is brightest value and 0 is the darkest
pub const rgba255 = struct {
    r: u32,
    g: u32,
    b: u32,
    a: u32,
};

pub fn rgba255to1s(color: rgba255) rgba1 {
    const rf = srgbToLinear(@as(f32, @floatFromInt(color.r)) / 255.0);
    const gf = srgbToLinear(@as(f32, @floatFromInt(color.g)) / 255.0);
    const bf = srgbToLinear(@as(f32, @floatFromInt(color.b)) / 255.0);

    return rgba1{ .r = rf, .g = gf, .b = bf, .a = @as(f32, @floatFromInt(color.a)) / 255.0 };
}

pub fn rgba255to1f(color: rgba255) [4]f32 {
    const rf = srgbToLinear(@as(f32, @floatFromInt(color.r)) / 255.0);
    const gf = srgbToLinear(@as(f32, @floatFromInt(color.g)) / 255.0);
    const bf = srgbToLinear(@as(f32, @floatFromInt(color.b)) / 255.0);

    return [4]f32{ rf, gf, bf, @as(f32, @floatFromInt(color.a)) / 255.0 };
}

fn srgbToLinear(c: f32) f32 {
    if (c <= 0.04045) {
        return c / 12.92;
    } else return std.math.pow(f32, (c + 0.055) / 1.055, 2.4);
}
