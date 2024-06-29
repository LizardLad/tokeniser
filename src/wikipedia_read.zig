const std = @import("std");
//const libxml2 = @cImport({
//    @cInclude("libxml/xmlreader.h");
//});
//const testing = std.testing;
const libxml2 = @cImport({
    @cInclude("libxml/xmlreader.h");
});

pub fn init() void {
    //return;
    libxml2.xmlInitParser();
}

const FindTextTagError = std.mem.Allocator.Error || std.fs.File.ReadError;

pub fn find_text_tag(allocator: std.mem.Allocator, fp: std.fs.File) FindTextTagError!std.fs.File {
    var buffer: []u8 = allocator.alloc(u8, std.mem.page_size) catch @panic("Failed to allocate enough memory");
    defer allocator.free(buffer);

    const bytes_read = fp.read(buffer) catch |e| {
        return e;
    };

    for (0..bytes_read) |i| {
        if (bytes_read - i < 13) {
            break; //Read again
        }

        const possible_tag = buffer[i .. i + 13];
        if (std.mem.eql(u8, possible_tag, "<text bytes=\"")) {
            std.log.info("Found a tag at {}, {s}", .{ i, possible_tag });
            return fp;
        }
    }

    return fp;
}

//test "basic add functionality" {
//    try testing.expect(add(3, 7) == 10);
//}
