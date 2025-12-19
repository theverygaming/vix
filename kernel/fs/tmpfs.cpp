#include <algorithm>
#include <string.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <vix/fs/vfs.h>
#include <vix/initfn.h>
#include <vix/status.h>

struct tmpfs_node {
    struct vfs::vnode vnode;
    std::unordered_map<std::string, struct tmpfs_node *> children;
    std::vector<uint8_t> data;
};

status::Status<> tmpfs_open(struct vfs::vnode *vnode) {
    return status::StatusCode::OK;
}

status::Status<> tmpfs_close(struct vfs::vnode *vnode) {
    return status::StatusCode::OK;
}

status::StatusOr<size_t> tmpfs_read(
    struct vfs::vnode *vnode, size_t offset, void *data, size_t bytes_max
) {
    struct tmpfs_node *tmpfsnode = (struct tmpfs_node *)vnode;

    // not a file
    if (tmpfsnode->vnode.type != vfs::vnode_type::REGULAR) {
        return status::StatusCode::EGENERIC; // FIXME: proper errors
    }

    offset = std::min(offset, tmpfsnode->data.size());

    size_t read_size = std::min(bytes_max, tmpfsnode->data.size() - offset);

    memcpy(data, &tmpfsnode->data[offset], read_size);

    return read_size;
}

status::StatusOr<size_t>
tmpfs_write(struct vfs::vnode *vnode, size_t offset, void *data, size_t bytes) {
    struct tmpfs_node *tmpfsnode = (struct tmpfs_node *)vnode;

    // not a file
    if (tmpfsnode->vnode.type != vfs::vnode_type::REGULAR) {
        return status::StatusCode::EGENERIC; // FIXME: proper errors
    }

    tmpfsnode->data.resize(std::max(offset + bytes, tmpfsnode->data.size()));

    tmpfsnode->vnode.attrs.sz_bytes = offset + bytes;
    tmpfsnode->vnode.attrs.sz_blocks = offset + bytes;

    memcpy(&tmpfsnode->data[offset], data, bytes);

    return bytes;
}

status::StatusOr<struct vfs::vnode *>
tmpfs_lookup(struct vfs::vnode *vnode, const char *name) {
    struct tmpfs_node *tmpfsnode = (struct tmpfs_node *)vnode;

    // not a directory (the user asked e.g. /dir/dir/file/something)
    if (tmpfsnode->vnode.type != vfs::vnode_type::DIR) {
        return status::StatusCode::EGENERIC; // FIXME: proper errors
    }

    auto name_stdstr = std::string(name);
    if (tmpfsnode->children.contains(std::string(name))) {
        return (struct vfs::vnode *)tmpfsnode->children[name_stdstr];
    }

    return status::StatusCode::EGENERIC; // FIXME: proper errors
}

status::StatusOr<struct vfs::vnode *> tmpfs_create(
    struct vfs::vnode *parent, const char *name, vfs::vnode_type type
) {
    // parent must be a directory
    if (parent->type != vfs::vnode_type::DIR) {
        return status::StatusCode::EGENERIC; // FIXME: proper errors
    }

    struct tmpfs_node *tmpfsparent = (struct tmpfs_node *)parent;

    struct tmpfs_node *node = new struct tmpfs_node({
        .refcount = 1,
        .ino = 124, // FIXME: I MADE IT THE FUCK UP
        .attrs =
            {
                .sz_bytes = 0,
                .sz_blocks = 0,
            },
        .type = type,
        .vfs = parent->vfs,
        .vfs_mounted = nullptr,
        .ops = parent->ops,
    });

    tmpfsparent->children[std::string(name)] = node;

    return (struct vfs::vnode *)node;
}

static struct vfs::vnodeops tmpfs_node_ops{
    .open = tmpfs_open,
    .close = tmpfs_close,
    .read = tmpfs_read,
    .write = tmpfs_write,
    .lookup = tmpfs_lookup,
    .create = tmpfs_create,
};

status::StatusOr<struct vfs::vfs *> tmpfs_mount(
    struct vfs::vfsops *ops,
    struct vfs::vnode *mountpoint,
    struct vfs::vnode *dev
) {
    struct vfs::vnode *root = (struct vfs::vnode *)new struct tmpfs_node({
        .refcount = 1,
        .ino = 123, // FIXME: I MADE IT THE FUCK UP
        .attrs =
            {
                .sz_bytes = 0,
                .sz_blocks = 0,
            },
        .type = vfs::vnode_type::DIR,
        .vfs = nullptr,
        .vfs_mounted = nullptr,
        .ops = &tmpfs_node_ops,
    });
    struct vfs::vfs *vfs = new struct vfs::vfs({
        .ops = ops,
        .root = root,
        .mounted_on = nullptr,
    });
    root->vfs = vfs;

    return vfs;
}

status::Status<> tmpfs_unmount(struct vfs::vfs *vfs) {
    // FIXME: check refcounts
    if (vfs->root->refcount != 1) {
        return status::StatusCode::EGENERIC; // FIXME: proper error code.. ?
    }
    delete vfs->root;
    delete vfs;
    return status::StatusCode::OK;
}

status::Status<> tmpfs_sync(struct vfs::vfs *vfs) {
    return status::StatusCode::OK;
}

static struct vfs::vfsops tmpfs_ops{
    .mount = tmpfs_mount,
    .unmount = tmpfs_unmount,
    .sync = tmpfs_sync,
};

struct vfs::fsdriver tmpfs_driver{
    .name = "tmpfs",
    .ops = &tmpfs_ops,
};

static void tmpfs_init() {
    vfs::register_driver(&tmpfs_driver);
}

INITFN_DEFINE(tmpfs, INITFN_SUPER_EARLY_DRIVER_INIT, 0, tmpfs_init);
