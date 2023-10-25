#include <debug.h>
#include <fs/path.h>
#include <fs/vfs.h>
#include <memory>
#include <mm/kmalloc.h>
#include <mutex>
#include <vector>

static std::shared_ptr<struct fs::vfs::node> root;

void fs::vfs::init() {
    root = std::shared_ptr<struct fs::vfs::node>(new struct fs::vfs::node);
    root->type = fs::vfs::node::type::DIRECTORY;
    root->name = "";

    struct fs::vfs::node *node = new struct fs::vfs::node;
    node->name = "dir 1";
    node->type = fs::vfs::node::type::DIRECTORY;
    root->children.push_back(std::shared_ptr<struct fs::vfs::node>(node));

    node = new struct fs::vfs::node;
    node->name = "file 1";
    node->type = fs::vfs::node::type::REGULAR_FILE;
    root->children.push_back(std::shared_ptr<struct fs::vfs::node>(node));

    node = new struct fs::vfs::node;
    node->type = fs::vfs::node::type::DIRECTORY;
    node->name = "file 3";

    struct fs::vfs::node *node2 = new struct fs::vfs::node;
    node2->name = "another file";
    node2->type = fs::vfs::node::type::REGULAR_FILE;
    node->children.push_back(std::shared_ptr<struct fs::vfs::node>(node2));

    root->children.push_back(std::shared_ptr<struct fs::vfs::node>(node));
}

static std::shared_ptr<struct fs::vfs::node> get_node(std::vector<std::string> &path) {
    std::shared_ptr<struct fs::vfs::node> current = root;
    for (size_t i = 0; i < path.size(); i++) {
        bool found = false;
        kprintf(KP_INFO, "a: %s\n", path[i].c_str());
        for (size_t j = 0; j < current->children.size(); j++) {
            if (current->children[j]->name == path[i]) {
                found = true;
                current = current->children[j];
                break;
            }
        }
        if (!found) {
            kprintf(KP_INFO, "could not find: %s\n", path[i].c_str());
            current = std::shared_ptr<struct fs::vfs::node>(nullptr);
            break;
        }
    }
    return current;
}

static void print_node(std::shared_ptr<struct fs::vfs::node> node, size_t depth) {
    std::string spaces;
    spaces.resize(depth * 4);
    for (size_t i = 0; i < depth * 4; i++) {
        spaces[i] = ' ';
    }

    kprintf(KP_INFO, "%s-> VFS node \"%s\"\n", spaces.c_str(), node->name.c_str());
    if (node->type == fs::vfs::node::type::DIRECTORY) {
        for (size_t i = 0; i < node->children.size(); i++) {
            print_node(node->children[i].get(), depth + 1);
        }
    }
}

void fs::vfs::print_tree() {
    sched::disable();
    kprintf(KP_INFO, "VFS tree:\n");
    print_node(root, 0);
    KERNEL_PANIC("done");
}

void fs::vfs::mount_fs(struct fsinfo fs, std::string mountpoint) {
    //kprintf(KP_INFO, "VFS tree:\n");
    //print_node(root, 0);
    std::vector<std::string> mp = fs::path::split_path(mountpoint);
    std::shared_ptr<struct fs::vfs::node> mountnode = get_node(mp);
    if (mountnode.get() == nullptr) {
        return;
    }
    if (mountnode->type != fs::vfs::node::type::DIRECTORY) {
        return;
    }
    mountnode->children.clear();
    mountnode->fs = new struct fsinfo(fs);
    //kprintf(KP_INFO, "VFS tree:\n");
    //print_node(root, 0);
}

fs::vfs::file *fs::vfs::fopen(std::string path) {
    /*
    std::vector<std::string> split_path = fs::path::split_path(path);
    ssize_t best_depth = -1;
    struct fs::vfs::fsinfo *best_mountpoint = nullptr;

    for (size_t i = 0; i < mountpoints.size(); i++) {
        if (fs::path::starts_with(&split_path, &mountpoints[i].mount_path)) {
            if ((ssize_t)mountpoints[i].mount_path.size() >= best_depth) {
                best_depth = mountpoints[i].mount_path.size();
                best_mountpoint = &mountpoints[i];
            }
        }
    }

    if (best_depth == -1) {
        DEBUG_PRINTF("no mountpoint for %s\n", fs::path::unsplit_path(split_path).c_str());
        return nullptr;
    }

    split_path.erase(0, best_mountpoint->mount_path.size() - 1);

    void *internal = best_mountpoint->fopen(best_mountpoint->info, &split_path);
    if (internal == nullptr) {
        DEBUG_PRINTF("no file for %s\n", fs::path::unsplit_path(split_path).c_str());
        return nullptr;
    }

    fs::vfs::file *f = new fs::vfs::file;
    f->mount = *best_mountpoint;
    f->internal_file = internal;
    return f;*/
    return nullptr;
}

void fs::vfs::fclose(file *file) {
    /*
    if (file == nullptr) {
        return;
    }
    file->mount.fclose(file->mount.info, file->internal_file);
    delete file;*/
}

size_t fs::vfs::fread(file *file, void *buf, size_t count) {
    /*
    return file->mount.fread(file->mount.info, file->internal_file, buf, count);*/
    return 0;
}

size_t fs::vfs::ftell(file *file) {
    /*
    return file->mount.ftell(file->mount.info, file->internal_file);*/
    return 0;
}

void fs::vfs::fseek(file *file, size_t pos, unsigned int flags) {
    /*
    file->mount.fseek(file->mount.info, file->internal_file, pos, flags);*/
}

bool fs::vfs::fptr(const char *path, void **fileptr) {
    file *file = fopen(path);
    if (file == nullptr) {
        return false;
    }
    fseek(file, 0, VFS_SEEK_END);
    size_t filesize = ftell(file);
    fseek(file, 0);
    *fileptr = mm::kmalloc(filesize + 30000); // FIXME: +30000 because kernel module code is broken and i need to sleep
    fread(file, *fileptr, filesize);
    fclose(file);
    return true;
}
