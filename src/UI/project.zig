const std = @import("std");
const zgui = @import("zgui");

pub const Project = struct {
    pub fn init() Project {
        return Project{};
    }

    pub fn draw(self: *Project) !void {
        _ = self;
        if (zgui.begin("Project", .{})) {
            if (zgui.button("Add Image(s)", .{})) {}
        }
        zgui.sameLine(.{});
        zgui.text("Image Count: {d}", .{3});

        if (zgui.beginTable("##ImageList", .{ .column = 2, .flags = .{ .row_bg = true, .scroll_y = true, .hideable = false } })) {
            zgui.tableSetupColumn("Image Name", .{ .flags = .{ .width_stretch = true } });
            zgui.tableSetupColumn("Type", .{ .flags = .{} });
            zgui.tableHeadersRow();
            var i: usize = 0;
            while (i < 3) : (i += 1) {
                zgui.tableNextRow(.{});
                _ = zgui.tableSetColumnIndex(0);

                zgui.textUnformatted("test_image.png");
                _ = zgui.tableSetColumnIndex(1);
                zgui.textUnformatted("light");
            }
            zgui.endTable();
        }

        zgui.end();
    }
};
