#include <cppstd/mutex.h>
#include <cppstd/vector.h>
#include <fs/path.h>
#include <fs/vfs.h>

static std::vector<struct fs::vfs::fsinfo> mountpoints;

void fs::vfs::mount_fs(struct fsinfo *fs, std::string *mountpoint) {
    struct fsinfo info = *fs;
    info.mount_path = fs::path::split_path(mountpoint);
}

fs::vfs::file *fs::vfs::fopen(std::string *path) {
    std::vector<std::string> split_path = fs::path::split_path(path);
    int maxdepth = -1;
    struct fs::vfs::fsinfo best_mountpoint;
}

void fs::vfs::fclose(file *file) {}

size_t fs::vfs::fread(file *file, void *buf, size_t count) {}

size_t fs::vfs::ftell(file *file) {}

void fs::vfs::fseek(file *file, size_t pos, unsigned int flags) {}
