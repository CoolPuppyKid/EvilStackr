pub const Image = struct {
    filename: []const u8,
    data: []u8,
    itype: ImageType,
    width: u32,
    height: u32,
    // dont ask why this is a u32
    channels: u32,
};

pub const ImageType = enum(u8) {
    LIGHT = 0,
    DARK = 1,
    BIAS = 2,
    FLAT = 3,
};
