#include <string.h>
#include <unordered_map>
#include <vix/debug.h>
#include <vix/fs/vfs.h>
#include <vix/kprintf.h>
#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/sched.h>
#include <vix/status.h>

// finds the last mountpoint on a given node, if there are mountpoints
static std::shared_ptr<struct vfs::vnode>
find_last_mount(std::shared_ptr<struct vfs::vnode> node) {
    while (node->vfs_mounted != nullptr) {
        node = node->vfs_mounted->root;
    }
    return node;
}

struct hash_cstring {
    size_t operator()(const char *str) const noexcept {
        size_t hash = 0;
        while (*str != '\0') {
            hash += *(str++);
        }
        return hash;
    }
};

struct cmp_cstring {
    bool operator()(const char *const a, const char *const b) const noexcept {
        return strcmp(a, b) == 0;
    }
};

namespace vfs {
    static std::shared_ptr<struct vnode> root;
    static std::unordered_map<
        const char *,
        struct fsdriver *,
        hash_cstring,
        cmp_cstring>
        drivers;

    void mount_root() {
        root = new struct vnode({
            .ino = 1, // FIXME: I made it the FUCK UP
            .attrs =
                {
                    .sz_bytes = 0,
                    .sz_blocks = 0,
                },
            .type = vnode_type::DIR,
            .vfs = nullptr,
            .vfs_mounted = nullptr,
            .ops = nullptr,
        });

        PANIC_IF_ERROR(mount("/", "tmpfs", nullptr));
    }

    void register_driver(struct fsdriver *drv) {
        drivers[drv->name] = drv;
    }

    void remove_driver(struct fsdriver *drv) {
        drivers.erase(drv->name);
    }

    status::Status<> mount(
        struct vfsops *ops,
        std::shared_ptr<struct vnode> mountpoint,
        std::shared_ptr<struct vnode> dev
    ) {
        if (mountpoint->vfs_mounted != nullptr) {
            KERNEL_PANIC("mount skill issue");
        }
        struct vfs *vfs;
        ASSIGN_OR_RETURN(vfs, ops->mount(ops, mountpoint, dev));
        mountpoint->vfs_mounted = vfs;
        vfs->mounted_on = mountpoint;
        return status::StatusCode::OK;
    }

    status::Status<> mount(
        const char *mountpoint,
        const char *fsname,
        std::shared_ptr<struct vnode> dev
    ) {
        if (drivers.contains(fsname)) {
            std::shared_ptr<struct vnode> mp;
            ASSIGN_OR_RETURN(mp, lookup(mountpoint));
            return mount(drivers[fsname]->ops, mp, dev);
        }
        return status::StatusCode::
            EGENERIC; // FIXME: error code -> should be fs type not found
    }

    status::StatusOr<std::shared_ptr<struct vnode>> lookup(const char *path) {
        return lookup(root, path);
    }

    status::StatusOr<std::shared_ptr<struct vnode>>
    lookup(std::shared_ptr<struct vnode> start, const char *path) {
        size_t pathlen = strlen(path);
        char *pathbuf = new char[pathlen + 1];
        memcpy(pathbuf, path, pathlen + 1);

        // replace slashes with null terminators to split the path into it's segments
        for (size_t i = 0; i < pathlen; i++) {
            if (pathbuf[i] == '/') {
                pathbuf[i] = '\0';
            }
        }

        std::shared_ptr<struct vnode> node = find_last_mount(start);

        for (size_t i = 0; i < pathlen; i++) {
            // empty path segment (e.g. double slash) -> ignore
            if (pathbuf[i] == '\0') {
                continue;
            }

            // not a directory (the user asked e.g. /dir/dir/file/something)
            if (node->type != vnode_type::DIR) {
                return status::StatusCode::EGENERIC; // FIXME: proper errors
            }

            char *segment = &pathbuf[i];
            size_t segment_len = strlen(segment);

            // TODO: handle . and ..

            auto lookup_ret = node->ops->lookup(node, segment);
            if (!lookup_ret.status().ok()) {
                // FIXME: cleanup
                delete[] pathbuf;
                return lookup_ret.status();
            }
            node = lookup_ret.value();

            node = find_last_mount(node);

            i += segment_len;
        }

        // FIXME: cleanup
        delete[] pathbuf;

        return node;
    }

    status::StatusOr<std::shared_ptr<struct vnode>> open(const char *path) {
        return open(root, path);
    }

    status::StatusOr<std::shared_ptr<struct vnode>>
    open(std::shared_ptr<struct vnode> start, const char *path) {
        std::shared_ptr<struct vnode> node;
        ASSIGN_OR_RETURN(node, lookup(path));
        RETURN_IF_ERROR(node->ops->open(node));
        return node;
    }

    status::Status<> close(std::shared_ptr<struct vnode> vnode) {
        return vnode->ops->close(vnode);
    }

    status::StatusOr<size_t> read(
        std::shared_ptr<struct vnode> vnode,
        size_t offset,
        void *data,
        size_t bytes_max
    ) {
        return vnode->ops->read(vnode, offset, data, bytes_max);
    }

    status::StatusOr<size_t> write(
        std::shared_ptr<struct vnode> vnode,
        size_t offset,
        void *data,
        size_t bytes
    ) {
        return vnode->ops->write(vnode, offset, data, bytes);
    }

    // FIXME: on create check if file exists!! else sounds like a pretty damn bad idea!
    // FIXME: also "." and ".." files shall never be created sob

    status::StatusOr<std::shared_ptr<struct vnode>> create(
        std::shared_ptr<struct vnode> parent, const char *name, vnode_type type
    ) {
        return parent->ops->create(parent, name, type);
    }

    status::StatusOr<std::shared_ptr<struct vnode>>
    create(const char *path, vnode_type type) {
        // TODO: lookup should probably have a way to just simply get the parent.. then we wouldn't need this mess

        size_t pathlen = strlen(path);
        char *pathbuf = new char[pathlen + 1];
        memcpy(pathbuf, path, pathlen + 1);

        // strip trailing slashes
        while (pathlen > 0 && pathbuf[pathlen - 1] == '/') {
            pathbuf[pathlen - 1] = '\0';
            pathlen--;
        }

        // find the filename (of the file to be created)
        while (pathlen > 0 && pathbuf[pathlen - 1] != '/') {
            pathlen--;
        }

        pathbuf[pathlen - 1] = '\0';

        const char *p1 = pathbuf;
        const char *p2 = &pathbuf[pathlen];

        // silly case: paths like "usr/" will result in p1 == p2
        // in that case we just set p1 to /
        if (p1 == p2) {
            p1 = "/";
            // and pathlen == 0 so we fix that (random value that's not 0, it's not used anymore)
            pathlen = 1;
        }

        if (pathlen == 0) {
            delete[] pathbuf;
            return status::StatusCode::EGENERIC; // FIXME: proper errors
        }

        auto lookup_ret = lookup(p1);

        if (!lookup_ret.status().ok()) {
            delete[] pathbuf;
            return lookup_ret.status();
        }

        auto create_ret = create(lookup_ret.value(), p2, type);
        delete[] pathbuf;

        if (!create_ret.status().ok()) {
            return create_ret.status();
        }

        return create_ret.value();
    }

    status::Status<>
    ioctl(std::shared_ptr<struct vnode> vnode, unsigned int cmd, void *arg) {
        return vnode->ops->ioctl(vnode, cmd, arg);
    }

    status::StatusOr<size_t> getdents(
        std::shared_ptr<struct vnode> vnode,
        struct dirent *buf,
        size_t buf_max,
        size_t *offset
    ) {
        return vnode->ops->getdents(vnode, buf, buf_max, offset);
    }
}

bool fs::vfs::fptr(const char *path, void **fileptr) {
    auto res_open = ::vfs::open(path);
    if (!res_open.status().ok()) {
        return false;
    }
    auto node = res_open.value();

    size_t filesize = node->attrs.sz_bytes;
    *fileptr = mm::kmalloc(
        filesize + 30000
    ); // FIXME: +30000 because kernel module code is broken and i need to sleep
    auto res_read = ::vfs::read(node, 0, *fileptr, filesize);
    if (!res_read.status().ok()) {
        PANIC_IF_ERROR(::vfs::close(node));
        mm::kfree(*fileptr);
        *fileptr = nullptr;
        return false;
    }
    PANIC_IF_ERROR(::vfs::close(node));
    return true;
}
