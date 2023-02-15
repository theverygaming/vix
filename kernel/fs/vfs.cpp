#include <cppstd/mutex.h>
#include <cppstd/vector.h>
#include <fs/path.h>
#include <fs/vfs.h>
#include <mm/kmalloc.h>

static std::vector<struct fs::vfs::fsinfo> mountpoints;

void fs::vfs::mount_fs(struct fsinfo *fs, std::string *mountpoint) {
    struct fsinfo info = *fs;
    info.mount_path = fs::path::split_path(mountpoint);
}

fs::vfs::file *fs::vfs::fopen(std::string *path) {
    std::vector<std::string> split_path = fs::path::split_path(path);
    int maxdepth = -1;
    struct fs::vfs::fsinfo best_mountpoint;

    fs::vfs::file *f = (fs::vfs::file *)mm::kmalloc(sizeof(fs::vfs::file));

    return f;
}

void fs::vfs::fclose(file *file) {}

size_t fs::vfs::fread(file *file, void *buf, size_t count) {
    return 0;
}

size_t fs::vfs::ftell(file *file) {
    return 0;
}

void fs::vfs::fseek(file *file, size_t pos, unsigned int flags) {}

bool fs::vfs::fptr(const char *path, void **fileptr) {
    std::string path_str = path;
    printf("path: %s\n", path_str.c_str());
    file *file = fopen(&path_str);
    fseek(file, 0, VFS_SEEK_END);
    size_t filesize = ftell(file);
    fseek(file, 0);
    *fileptr = mm::kmalloc(filesize);
    fread(file, *fileptr, filesize);
    fclose(file);
    return false;
}
