const std = @import("std");
const wikipedia = @import("wikipedia_read.zig");

fn get_relative_path(allocator: std.mem.Allocator, rel_path: []const u8) []u8 {
    const cwd = std.fs.cwd().realpathAlloc(allocator, ".") catch @panic("Could not get current working directory");
    defer allocator.free(cwd);

    const file_path_resolver = [_][]const u8{ cwd, rel_path };
    const resolved_path = std.fs.path.resolve(allocator, &file_path_resolver) catch @panic("Unable to resolve path");
    return resolved_path;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer {
        const deinit_status = gpa.deinit();
        //fail test; can't try in defer as defer is executed after we return
        if (deinit_status == .leak) std.testing.expect(false) catch @panic("There is a memory leak");
    }
    const allocator = gpa.allocator();

    var args = try std.process.argsWithAllocator(allocator);
    defer args.deinit();
    if (!args.skip()) {
        std.log.err("No arguments provided, including the executable name", .{});
        std.process.exit(1);
    }

    //Get the path from the cmd line arguments
    const file_path_opt = args.next();
    if (file_path_opt == null) {
        std.log.err("No file path provided", .{});
        std.process.exit(1);
    }

    //Get the absolute path from the relative path
    const resolved_path = get_relative_path(allocator, file_path_opt.?);
    defer allocator.free(resolved_path);
    std.log.debug("Resolved path: {s}", .{resolved_path});

    //Open the file
    var fp = std.fs.openFileAbsolute(resolved_path, .{ .mode = std.fs.File.OpenMode.read_only }) catch @panic("Failed to open file");

    //Read from the file into a buffer
    var buff: [100]u8 = undefined;
    const bytes_read = fp.read(&buff) catch |err| blk: {
        std.log.warn("Could not read from file {}", .{err});
        break :blk 0;
    };
    std.log.info("Read {} bytes, {s}", .{ bytes_read, buff });

    wikipedia.init();
    _ = wikipedia.find_text_tag(allocator, fp) catch @panic("Failed to read from file");
}

//test "simple test" {
//}
