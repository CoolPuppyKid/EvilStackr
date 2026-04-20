pub const Image = struct {
    filename: []const u8,
    data: []u8,
    itype: ImageType,
};

pub const ImageType = enum(u8) {
    LIGHT = 0,
    DARK = 1,
    BIAS = 2,
    FLAT = 3,
};
