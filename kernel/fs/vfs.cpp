#include <cppstd/mutex.h>
#include <cppstd/vector.h>
#include <fs/path.h>
#include <fs/vfs.h>

/* the VFS is just a list of mount points an their paths, this will have to be replaced with something a bit better at some point... */

std::mutex mountpoint_lock;
std::vector<struct fs::vfs::vfs_mountpoint> mountpoints;

void fs::vfs::mount(struct vfs_mountpoint mount) {
    mountpoint_lock.lock();
    mountpoints.push_back(mount);
    mountpoint_lock.unlock();
}

void fs::vfs::unmount(char *mountpath) {
    mountpoint_lock.lock();
    for (size_t i = 0; i < mountpoints.size(); i++) {
        if (fs::path::path_compare(mountpath, mountpoints[i].mountpath)) {
            mountpoints.erase(i);
            break;
        }
    }
    mountpoint_lock.unlock();
}

static int find_best_mountpoint_index(char *path) {
    int bestIndex = -1;
    int bestDepth = -1;
    for (size_t i = 0; i < mountpoints.size(); i++) {
        if (fs::path::startswith_path(mountpoints[i].mountpath, path)) {
            int depth = fs::path::path_depth(mountpoints[i].mountpath);
            if (depth > bestDepth) {
                bestIndex = i;
                bestDepth = depth;
            }
        }
    }
    return bestIndex;
}

size_t fs::vfs::fsize(char *path) {
    mountpoint_lock.lock();
    int bestindex = find_best_mountpoint_index(path);
    if (bestindex < 0) {
        mountpoint_lock.unlock();
        return 0;
    }
    char *rmprefix = fs::path::rm_prefix(mountpoints[bestindex].mountpath, path);
    size_t size = mountpoints[bestindex].fsize(rmprefix);
    memalloc::single::kfree(rmprefix);
    mountpoint_lock.unlock();
    return size;
}

bool fs::vfs::fload(char *path, void *memloc) {
    mountpoint_lock.lock();
    int bestindex = find_best_mountpoint_index(path);
    if (bestindex < 0) {
        mountpoint_lock.unlock();
        return false;
    }
    char *rmprefix = fs::path::rm_prefix(mountpoints[bestindex].mountpath, path);
    bool loaded = mountpoints[bestindex].fload(rmprefix, memloc);
    memalloc::single::kfree(rmprefix);
    mountpoint_lock.unlock();
    return loaded;
}

bool fs::vfs::fptr(char *path, void **fileptr) {
    mountpoint_lock.lock();
    int bestindex = find_best_mountpoint_index(path);
    if (bestindex < 0) {
        mountpoint_lock.unlock();
        return false;
    }
    char *rmprefix = fs::path::rm_prefix(mountpoints[bestindex].mountpath, path);
    bool loaded = mountpoints[bestindex].fptr(rmprefix, fileptr);
    memalloc::single::kfree(rmprefix);
    mountpoint_lock.unlock();
    return loaded;
}
