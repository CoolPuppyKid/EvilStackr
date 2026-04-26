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

const LoadResult = struct {
    img: image.Image,
    owned_path: []const u8,
};

pub const Project = struct {
    pub fn init(allocator: std.mem.Allocator) !Project {
        const loadedImages = try std.ArrayList(image.Image).initCapacity(allocator, 1);
        return Project{
            .allocator = allocator,
            .loadedImages = loadedImages,
            .loading_total = std.atomic.Value(u32).init(0),
            .loading_completed = std.atomic.Value(u32).init(0),
            .is_loading = std.atomic.Value(bool).init(false),
            .result_mutex = .{},
            .pending_results = std.ArrayList(LoadResult).initCapacity(allocator, 1) catch unreachable,
            .owned_paths = std.ArrayList([]const u8).initCapacity(allocator, 1) catch unreachable,
        };
    }

    pub fn deinit(self: *Project) void {
        std.debug.print("Freeing allocated data, This may take awhile for large projects.\n", .{});
        for (self.loadedImages.items) |img| {
            self.allocator.free(img.data);
        }
        self.loadedImages.deinit(self.allocator);
        for (self.owned_paths.items) |p| self.allocator.free(p);
        self.owned_paths.deinit(self.allocator);
        self.pending_results.deinit(self.allocator);
    }

    fn startAddImages(self: *Project) void {
        var imagePaths = nfd.openFilesDialog("*", null) catch return;
        defer nfd.freePaths(&imagePaths);

        if (imagePaths.items.len == 0) return;

        for (self.owned_paths.items) |p| self.allocator.free(p);
        self.owned_paths.clearRetainingCapacity();
        for (imagePaths.items) |p| {
            self.owned_paths.append(self.allocator, p) catch continue;
        }

        self.loading_total.store(@intCast(self.owned_paths.items.len), .release);
        self.loading_completed.store(0, .release);
        self.is_loading.store(true, .release);

        const thread = std.Thread.spawn(.{}, loadImagesThread, .{self}) catch return;
        thread.detach();
    }

    fn loadImagesThread(self: *Project) void {
        for (self.owned_paths.items) |path| {
            const fileName = std.fs.path.basename(path);
            std.debug.print("Attempting to load: {s}\n", .{fileName});

            var img = image.Image{ .filename = fileName, .itype = .LIGHT, .data = undefined, .width = 0, .height = 0, .channels = 0 };
            var loaded = false;

            var stbiImage: ?zigstbi.Image = zigstbi.load_file(path, 0) catch |err| blk: {
                std.debug.print("STBI failed: {s}, trying LibRaw\n", .{@errorName(err)});
                break :blk null;
            };

            if (stbiImage != null) {
                const owned = self.allocator.dupe(u8, stbiImage.?.bytes) catch {
                    stbiImage.?.deinit();
                    _ = self.loading_completed.fetchAdd(1, .release);
                    continue;
                };
                img.data = owned;
                img.width = stbiImage.?.width;
                img.height = stbiImage.?.height;
                img.channels = stbiImage.?.channel_number;
                stbiImage.?.deinit();
                loaded = true;
            } else {
                const librawData = c.libraw_init(0) orelse {
                    _ = self.loading_completed.fetchAdd(1, .release);
                    continue;
                };
                defer c.libraw_close(librawData);

                librawData.*.params.output_bps = 16;
                librawData.*.params.use_camera_wb = 1;
                librawData.*.params.no_auto_bright = 1;
                librawData.*.params.gamm[0] = 1.0;
                librawData.*.params.gamm[1] = 1.0;

                if (c.libraw_open_file(librawData, path.ptr) != c.LIBRAW_SUCCESS) {
                    std.debug.print("LibRaw failed to open: {s}\n", .{fileName});
                    _ = self.loading_completed.fetchAdd(1, .release);
                    continue;
                }
                if (c.libraw_unpack(librawData) != c.LIBRAW_SUCCESS) {
                    _ = self.loading_completed.fetchAdd(1, .release);
                    continue;
                }
                if (c.libraw_dcraw_process(librawData) != c.LIBRAW_SUCCESS) {
                    _ = self.loading_completed.fetchAdd(1, .release);
                    continue;
                }

                var errcode: c_int = 0;
                const processed = c.libraw_dcraw_make_mem_image(librawData, &errcode);
                if (errcode != c.LIBRAW_SUCCESS or processed == null) {
                    _ = self.loading_completed.fetchAdd(1, .release);
                    continue;
                }
                defer c.libraw_dcraw_clear_mem(processed);

                const byteLen = processed.*.data_size;
                const dataPtr: [*]u8 = @ptrCast(&processed.*.data);
                const owned = self.allocator.dupe(u8, dataPtr[0..byteLen]) catch {
                    _ = self.loading_completed.fetchAdd(1, .release);
                    continue;
                };
                img.data = owned;
                img.width = processed.*.width;
                img.height = processed.*.height;
                img.channels = processed.*.colors;
                loaded = true;
            }

            if (loaded) {
                self.result_mutex.lock();
                self.pending_results.append(self.allocator, .{ .img = img, .owned_path = path }) catch {};
                self.result_mutex.unlock();
            }
            _ = self.loading_completed.fetchAdd(1, .release);
        }
        self.is_loading.store(false, .release);
    }

    fn drainResults(self: *Project) void {
        self.result_mutex.lock();
        defer self.result_mutex.unlock();
        for (self.pending_results.items) |result| {
            self.loadedImages.append(self.allocator, result.img) catch continue;
        }
        self.pending_results.clearRetainingCapacity();
    }

    pub fn draw(self: *Project) !void {
        self.drainResults();

        const loading = self.is_loading.load(.acquire);

        if (zgui.begin("Project", .{})) {
            if (!loading) {
                if (zgui.button("Add Image(s)", .{})) {
                    self.startAddImages();
                }
            } else {
                zgui.textDisabled("Loading...", .{});
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

                zgui.text("{s} | {d}x{d}", .{ self.loadedImages.items[i].filename, self.loadedImages.items[i].width, self.loadedImages.items[i].height });
                _ = zgui.tableSetColumnIndex(1);
                zgui.textUnformatted(@tagName(self.loadedImages.items[i].itype));
            }
            zgui.endTable();
        }

        zgui.end();

        if (loading) {
            const completed = self.loading_completed.load(.acquire);
            const total = self.loading_total.load(.acquire);
            zgui.openPopup("Loading Images", .{});
            if (zgui.beginPopupModal("Loading Images", .{ .flags = .{ .no_resize = true, .always_auto_resize = true } })) {
                zgui.text("Loading {d} / {d} images...", .{ completed, total });
                const fraction = if (total > 0) @as(f32, @floatFromInt(completed)) / @as(f32, @floatFromInt(total)) else 0.0;
                zgui.progressBar(.{ .fraction = fraction, .overlay = "", .w = 300 });
                zgui.endPopup();
            }
        }
    }

    allocator: std.mem.Allocator,
    loadedImages: std.ArrayList(image.Image),
    loading_total: std.atomic.Value(u32),
    loading_completed: std.atomic.Value(u32),
    is_loading: std.atomic.Value(bool),
    result_mutex: std.Thread.Mutex,
    pending_results: std.ArrayList(LoadResult),
    owned_paths: std.ArrayList([]const u8),
};
