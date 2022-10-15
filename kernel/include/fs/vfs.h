#pragma once
#include <types.h>

namespace fs::vfs {
    struct vfs_mountpoint {
        char mountpath[100];
        uint16_t uid;
        uint16_t gid;
        size_t (*fsize)(char *path);              // get file size in bytes
        bool (*fload)(char *path, void *memloc);  // load file into memloc
        bool (*fptr)(char *path, void **fileptr); // give pointer to file in ram(if it's a ramfs)
    };

    void mount(struct vfs_mountpoint mount);
    void unmount(char *mountpath);

    size_t fsize(char *path);
    bool fload(char *path, void *memloc);
    bool fptr(char *path, void **fileptr);
}
