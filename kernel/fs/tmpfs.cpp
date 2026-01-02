#include <algorithm>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <vix/fs/vfs.h>
#include <vix/initfn.h>
#include <vix/status.h>

struct tmpfs_node {
    struct vfs::vnode vnode;
    std::unordered_map<std::string, std::shared_ptr<struct tmpfs_node>>
        children;
    std::vector<uint8_t> data;
};

status::Status<> tmpfs_open(std::shared_ptr<struct vfs::vnode> vnode) {
    return status::StatusCode::OK;
}

status::Status<> tmpfs_close(std::shared_ptr<struct vfs::vnode> vnode) {
    return status::StatusCode::OK;
}

status::StatusOr<size_t> tmpfs_read(
    std::shared_ptr<struct vfs::vnode> vnode,
    size_t offset,
    void *data,
    size_t bytes_max
) {
    auto tmpfsnode = std::static_pointer_cast<struct tmpfs_node>(vnode);

    // not a file
    if (tmpfsnode->vnode.type != vfs::vnode_type::REGULAR) {
        return status::StatusCode::EGENERIC; // FIXME: proper errors
    }

    offset = std::min(offset, tmpfsnode->data.size());

    size_t read_size = std::min(bytes_max, tmpfsnode->data.size() - offset);

    memcpy(data, &tmpfsnode->data[offset], read_size);

    return read_size;
}

status::StatusOr<size_t> tmpfs_write(
    std::shared_ptr<struct vfs::vnode> vnode,
    size_t offset,
    void *data,
    size_t bytes
) {
    auto tmpfsnode = std::static_pointer_cast<struct tmpfs_node>(vnode);

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

status::StatusOr<std::shared_ptr<struct vfs::vnode>>
tmpfs_lookup(std::shared_ptr<struct vfs::vnode> vnode, const char *name) {
    auto tmpfsnode = std::static_pointer_cast<struct tmpfs_node>(vnode);

    // not a directory (the user asked e.g. /dir/dir/file/something)
    if (tmpfsnode->vnode.type != vfs::vnode_type::DIR) {
        return status::StatusCode::EGENERIC; // FIXME: proper errors
    }

    auto name_stdstr = std::string(name);
    if (tmpfsnode->children.contains(std::string(name))) {
        return static_pointer_cast<struct vfs::vnode>(
            tmpfsnode->children[name_stdstr]
        );
    }

    return status::StatusCode::EGENERIC; // FIXME: proper errors
}

status::StatusOr<std::shared_ptr<struct vfs::vnode>> tmpfs_create(
    std::shared_ptr<struct vfs::vnode> parent,
    const char *name,
    vfs::vnode_type type
) {
    // parent must be a directory
    if (parent->type != vfs::vnode_type::DIR) {
        return status::StatusCode::EGENERIC; // FIXME: proper errors
    }

    auto tmpfsparent = std::static_pointer_cast<struct tmpfs_node>(parent);

    std::shared_ptr<struct tmpfs_node> node(new struct tmpfs_node({
        .vnode =
            {
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
            },
        .children = std::
            unordered_map<std::string, std::shared_ptr<struct tmpfs_node>>(),
        .data = std::vector<uint8_t>(),
    }));

    tmpfsparent->children[std::string(name)] = node;

    return std::static_pointer_cast<struct vfs::vnode>(node);
}

status::Status<> tmpfs_ioctl(
    std::shared_ptr<struct vfs::vnode> vnode, unsigned int cmd, void *arg
) {
    return status::StatusCode::EGENERIC; // FIXME: proper error code.. ?
}

status::StatusOr<size_t> tmpfs_getdents(
    std::shared_ptr<struct vfs::vnode> vnode,
    struct vfs::dirent *buf,
    size_t buf_max,
    size_t *offset
) {
    auto tmpfsnode = std::static_pointer_cast<struct tmpfs_node>(vnode);

    // not a directory (the user asked e.g. /dir/dir/file/something)
    if (tmpfsnode->vnode.type != vfs::vnode_type::DIR) {
        return status::StatusCode::EGENERIC; // FIXME: proper errors
    }

    auto it = tmpfsnode->children.begin();

    // advance iterator to offset (cursed because unordered_map but whatever)
    // FIXME: probably not safe when another thread inserts a file into the directory?
    for (size_t i = 0; it != tmpfsnode->children.end() && i != *offset;
         i++, it++) {}

    size_t bytes_written = 0;

    while (it != tmpfsnode->children.end()) {
        // dirent struct includes null terminator
        size_t bytes = sizeof(struct vfs::dirent) + it->first.size();
        // won't fit?
        if (buf_max < bytes) {
            break;
        }
        buf->reclen = bytes;
        buf->ino = tmpfsnode->vnode.ino;
        buf->type = tmpfsnode->vnode.type;
        memcpy(buf->name, it->first.c_str(), it->first.size() + 1);

        // FIXME: alignment?
        buf = (struct vfs::dirent *)((uint8_t *)buf + bytes);

        buf_max -= bytes;
        (*offset)++;
        bytes_written += bytes;
        it++;
    }
    return bytes_written;
}

static struct vfs::vnodeops tmpfs_node_ops{
    .open = tmpfs_open,
    .close = tmpfs_close,
    .read = tmpfs_read,
    .write = tmpfs_write,
    .lookup = tmpfs_lookup,
    .create = tmpfs_create,
    .ioctl = tmpfs_ioctl,
    .getdents = tmpfs_getdents,
};

status::StatusOr<struct vfs::vfs *> tmpfs_mount(
    struct vfs::vfsops *ops,
    std::shared_ptr<struct vfs::vnode> mountpoint,
    std::shared_ptr<struct vfs::vnode> dev
) {
    std::shared_ptr<struct tmpfs_node> root(new struct tmpfs_node({
        .vnode =
            {
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
            },
        .children = std::
            unordered_map<std::string, std::shared_ptr<struct tmpfs_node>>(),
        .data = std::vector<uint8_t>(),
    }));
    auto rootv = std::static_pointer_cast<struct vfs::vnode>(root);
    struct vfs::vfs *vfs = new struct vfs::vfs({
        .ops = ops,
        .root = rootv,
        .mounted_on = nullptr,
    });
    rootv->vfs = vfs;

    return vfs;
}

status::Status<> tmpfs_unmount(struct vfs::vfs *vfs) {
    if (vfs->root.use_count() != 1) {
        return status::StatusCode::EGENERIC; // FIXME: proper error code.. ?
    }
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
