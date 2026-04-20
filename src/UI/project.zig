const std = @import("std");
const zgui = @import("zgui");
const zigstbi = @import("zigstbi");
const nfd = @import("nfd");
const image = @import("../image.zig");
const c = @import("../c.zig").c;

const AddImageError = error{
    NfdError,
    OutOfMemory,
    LibrawInitFailed,
    LibrawOpenFailed,
    LibrawProcessFailed,
};

pub const Project = struct {
    pub fn init(allocator: std.mem.Allocator) !Project {
        const loadedImages = try std.ArrayList(image.Image).initCapacity(allocator, 1);
        return Project{ .allocator = allocator, .loadedImages = loadedImages };
    }

    pub fn deinit(self: *Project) void {
        self.loadedImages.deinit(self.allocator);
    }

    fn addImage(self: *Project) AddImageError!void {
        var imagePaths = try nfd.openFilesDialog("*", null);
        defer nfd.freePaths(&imagePaths);
        var i: usize = 0;
        while (i < imagePaths.items.len) : (i += 1) {
            if (!std.unicode.wtf8ValidateSlice(imagePaths.items[i])) {
                std.debug.print("Skipping invalid path at index {d}\n", .{i});
                continue;
            }
            const fileName = std.fs.path.basename(imagePaths.items[i]);
            std.debug.print("Attempting to load: {s}\n", .{fileName});

            var stbiImage: ?zigstbi.Image = zigstbi.load_file(imagePaths.items[i], 0) catch |err| blk: {
                std.debug.print("STBI failed to load image: {s}, This is normal if the image is a raw image.\n", .{@errorName(err)});
                break :blk null;
            };
            defer if (stbiImage != null) stbiImage.?.deinit();

            var img = image.Image{ .filename = fileName, .itype = .LIGHT, .data = undefined };

            if (stbiImage) |si| {
                img.data = si.bytes;
            } else {
                const librawData = c.libraw_init(0) orelse {
                    std.debug.print("Failed to init LibRaw\n", .{});
                    return error.LibrawInitFailed;
                };
                defer c.libraw_close(librawData);

                if (c.libraw_open_file(librawData, imagePaths.items[i].ptr) != c.LIBRAW_SUCCESS) {
                    std.debug.print("Failed to open image with LibRaw, therefore this file is not supported by EvilStackr\n", .{});
                    return error.LibrawOpenFailed;
                }

                if (c.libraw_unpack(librawData) != c.LIBRAW_SUCCESS) {
                    std.debug.print("Failed to unpack RAW image\n", .{});
                    return error.LibrawProcessFailed;
                }
                std.debug.print("Processing image data\n", .{});

                if (c.libraw_dcraw_process(librawData) != c.LIBRAW_SUCCESS) {
                    std.debug.print("Failed to process RAW image\n", .{});
                    return error.LibrawProcessFailed;
                }

                var errcode: c_int = 0;
                const processed = c.libraw_dcraw_make_mem_image(librawData, &errcode);
                if (errcode != c.LIBRAW_SUCCESS or processed == null) {
                    std.debug.print("Failed to extract image data\n", .{});
                    return error.LibrawProcessFailed;
                }
                defer c.libraw_dcraw_clear_mem(processed);
                std.debug.print("Copying image data\n", .{});

                const byteLen = processed.*.data_size;
                const dataPtr: [*]u8 = @ptrCast(&processed.*.data);
                const owned = try self.allocator.dupe(u8, dataPtr[0..byteLen]);
                img.data = owned;
            }

            try self.loadedImages.append(self.allocator, img);
        }
    }

    pub fn draw(self: *Project) !void {
        if (zgui.begin("Project", .{})) {
            if (zgui.button("Add Image(s)", .{})) {
                addImage(self) catch |err| {
                    switch (err) {
                        error.LibrawOpenFailed => {},
                        else => {},
                    }
                };
            }
        }
        zgui.sameLine(.{});
        zgui.text("Image Count: {d}", .{self.loadedImages.items.len});

        if (zgui.beginTable("##ImageList", .{ .column = 2, .flags = .{ .row_bg = true, .scroll_y = true, .hideable = false } })) {
            zgui.tableSetupColumn("Image Name", .{ .flags = .{ .width_stretch = true } });
            zgui.tableSetupColumn("Type", .{ .flags = .{} });
            zgui.tableHeadersRow();
            var i: usize = 0;
            while (i < self.loadedImages.items.len) : (i += 1) {
                zgui.tableNextRow(.{});
                _ = zgui.tableSetColumnIndex(0);

                zgui.textUnformatted(self.loadedImages.items[i].filename);
                _ = zgui.tableSetColumnIndex(1);
                zgui.textUnformatted(@tagName(self.loadedImages.items[i].itype));
            }
            zgui.endTable();
        }

        zgui.end();
    }

    allocator: std.mem.Allocator,
    loadedImages: std.ArrayList(image.Image),
};
