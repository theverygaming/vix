#include <debug.h>
#include <fs/path.h>
#include <fs/vfs.h>
#include <memory>
#include <mm/kmalloc.h>
#include <mutex>
#include <vector>

static std::vector<struct fs::vfs::fsinfo> mountpoints;

void fs::vfs::mount_fs(struct fsinfo fs, std::string mountpoint) {
    fs.mount_path = fs::path::split_path(&mountpoint);
    mountpoints.push_back(fs);
}

fs::vfs::file *fs::vfs::fopen(std::string path) {
    std::vector<std::string> split_path = fs::path::split_path(&path);
    size_t best_depth = 0;
    struct fs::vfs::fsinfo *best_mountpoint = nullptr;

    for (size_t i = 0; i < mountpoints.size(); i++) {
        if (fs::path::starts_with(&split_path, &mountpoints[i].mount_path)) {
            if (mountpoints[i].mount_path.size() >= best_depth) {
                best_depth = mountpoints[i].mount_path.size();
                best_mountpoint = &mountpoints[i];
            }
        }
    }

    if (best_depth == -1) {
        DEBUG_PRINTF("no mountpoint for %s\n", fs::path::unsplit_path(&split_path).c_str());
        return nullptr;
    }

    split_path.erase(0, best_mountpoint->mount_path.size() - 1);

    void *internal = best_mountpoint->fopen(best_mountpoint->info, &split_path);
    if (internal == nullptr) {
        DEBUG_PRINTF("no file for %s\n", fs::path::unsplit_path(&split_path).c_str());
        return nullptr;
    }

    fs::vfs::file *f = new fs::vfs::file;
    f->mount = *best_mountpoint;
    f->internal_file = internal;
    return f;
}

void fs::vfs::fclose(file *file) {
    if (file == nullptr) {
        return;
    }
    file->mount.fclose(file->mount.info, file->internal_file);
    delete file;
}

size_t fs::vfs::fread(file *file, void *buf, size_t count) {
    return file->mount.fread(file->mount.info, file->internal_file, buf, count);
}

size_t fs::vfs::ftell(file *file) {
    return file->mount.ftell(file->mount.info, file->internal_file);
}

void fs::vfs::fseek(file *file, size_t pos, unsigned int flags) {
    file->mount.fseek(file->mount.info, file->internal_file, pos, flags);
}

bool fs::vfs::fptr(const char *path, void **fileptr) {
    file *file = fopen(path);
    if (file == nullptr) {
        return false;
    }
    fseek(file, 0, VFS_SEEK_END);
    size_t filesize = ftell(file);
    fseek(file, 0);
    *fileptr = mm::kmalloc(filesize + 30000); // TODO: +30000 because kernel module code is broken and i need to sleep
    fread(file, *fileptr, filesize);
    fclose(file);
    return true;
}
