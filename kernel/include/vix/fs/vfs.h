#pragma once
#include <memory>
#include <vix/mm/kheap.h>
#include <vix/status.h>
#include <vix/types.h>

namespace vfs {
    typedef unsigned int ino_t;

    enum class vnode_type {
        REGULAR, // regular file
        DIR,     // directory
        CHRDEV,  // character device
        BLKDEV,  // block device
        //FIFO,
        //LINK,
        //SOCKET,
    };

    struct dirent {
        size_t reclen;
        ino_t ino;
        vnode_type type;
        char name[1]; // variable length
    };

    struct vnode_attrs {
        size_t sz_bytes;
        size_t sz_blocks;
    };

    struct vnode {
        ino_t ino;
        vnode_attrs attrs;
        vnode_type type;
        struct vfs *vfs;
        struct vfs *vfs_mounted;
        struct vnodeops *ops;
    };

    struct vnodeops {
        status::Status<> (*open)(std::shared_ptr<struct vnode> vnode);
        status::Status<> (*close)(std::shared_ptr<struct vnode> vnode);
        status::StatusOr<size_t> (*read)(
            std::shared_ptr<struct vnode> vnode,
            size_t offset,
            void *data,
            size_t bytes_max
        );
        status::StatusOr<size_t> (*write)(
            std::shared_ptr<struct vnode> vnode,
            size_t offset,
            void *data,
            size_t bytes
        );
        status::StatusOr<std::shared_ptr<struct vnode>> (*lookup)(
            std::shared_ptr<struct vnode> vnode, const char *name
        );
        status::StatusOr<std::shared_ptr<struct vnode>> (*create)(
            std::shared_ptr<struct vnode> parent,
            const char *name,
            vnode_type type
        );
        status::Status<> (*ioctl)(
            std::shared_ptr<struct vnode> vnode, unsigned int cmd, void *arg
        );
        status::StatusOr<size_t> (*getdents)(
            std::shared_ptr<struct vnode> vnode,
            struct dirent *buf,
            size_t buf_max,
            size_t *offset
        );
    };

    struct vfs {
        struct vfsops *ops;
        std::shared_ptr<struct vnode> root;
        std::shared_ptr<struct vnode> mounted_on;
    };

    struct vfsops {
        // mount the filesystem, allocate a vfs struct (this is the only point where one is ever allocated)
        status::StatusOr<struct vfs *> (*mount)(
            struct vfsops *ops,
            std::shared_ptr<struct vnode> mountpoint,
            std::shared_ptr<struct vnode> dev
        );
        // unmount the filesystem, clear a vfs struct for deallocation (this is the only point where one is ever deallocated)
        status::Status<> (*unmount)(struct vfs *vfs);
        // write all cached data to disk
        status::Status<> (*sync)(struct vfs *vfs);
    };

    struct fsdriver {
        const char *name;
        struct vfsops *ops;
        // TODO: probe n stuff sometime?
    };

    void mount_root();

    void register_driver(struct fsdriver *drv);
    void remove_driver(struct fsdriver *drv);

    status::Status<> mount(
        struct vfsops *ops,
        std::shared_ptr<struct vnode> mountpoint,
        std::shared_ptr<struct vnode> dev
    );
    status::Status<> mount(
        const char *mountpoint,
        const char *fsname,
        std::shared_ptr<struct vnode> dev
    );

    status::StatusOr<std::shared_ptr<struct vnode>> lookup(const char *path);
    status::StatusOr<std::shared_ptr<struct vnode>>
    lookup(std::shared_ptr<struct vnode> start, const char *path);

    status::StatusOr<std::shared_ptr<struct vnode>> open(const char *path);
    status::StatusOr<std::shared_ptr<struct vnode>>
    open(std::shared_ptr<struct vnode> start, const char *path);
    status::Status<> close(std::shared_ptr<struct vnode> vnode);

    status::StatusOr<size_t> read(
        std::shared_ptr<struct vnode> vnode,
        size_t offset,
        void *data,
        size_t bytes_max
    );
    status::StatusOr<size_t> write(
        std::shared_ptr<struct vnode> vnode,
        size_t offset,
        void *data,
        size_t bytes
    );

    status::StatusOr<std::shared_ptr<struct vnode>> create(
        std::shared_ptr<struct vnode> parent, const char *name, vnode_type type
    );
    status::StatusOr<std::shared_ptr<struct vnode>>
    create(const char *path, vnode_type type);

    status::Status<>
    ioctl(std::shared_ptr<struct vnode> vnode, unsigned int cmd, void *arg);

    // returns either error, or amount of bytes written to buffer (zero on end of directory!), offset
    // must be initialized to 0, and is required as an internal reference
    status::StatusOr<size_t> getdents(
        std::shared_ptr<struct vnode> vnode,
        struct dirent *buf,
        size_t buf_max,
        size_t *offset
    );
}

namespace fs::vfs {
    // FIXME: really old, but here because some things need it still
    bool fptr(const char *path, void **fileptr);
}
