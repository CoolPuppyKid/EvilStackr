const std = @import("std");

pub fn getPathOfFileAlloc(fileName: []const u8, allocator: std.mem.Allocator) ![]const u8 {
    const exePath = try std.fs.selfExeDirPathAlloc(allocator);
    defer allocator.free(exePath);

    const assetsPath = try std.fs.path.join(allocator, &[_][]const u8{ exePath, "assets", fileName });

    return assetsPath;
}
