#include <memory>
#include <mutex>
#include <vector>
#include <vix/debug.h>
#include <vix/fs/path.h>
#include <vix/fs/vfs.h>
#include <vix/mm/kheap.h>
#include <vix/sched.h>

#define TEST_NEW_VFS

#ifdef TEST_NEW_VFS
static std::shared_ptr<struct fs::vfs::inode> root;
#endif // TEST_NEW_VFS

void fs::vfs::init() {
#ifdef TEST_NEW_VFS
    root = std::shared_ptr<struct fs::vfs::inode>(new struct fs::vfs::inode);
    root->type = fs::vfs::inode::type::DIRECTORY;

    struct fs::vfs::dirent *de = new struct fs::vfs::dirent;
    de->name = "bin";
    de->inode = std::shared_ptr<struct fs::vfs::inode>(new struct fs::vfs::inode);
    de->inode->type = fs::vfs::inode::type::DIRECTORY;
    de->inode->direntries.push_back(std::shared_ptr<struct fs::vfs::dirent>(new struct fs::vfs::dirent));
    de->inode->direntries[0]->name = "sh";
    de->inode->direntries[0]->inode = std::shared_ptr<struct fs::vfs::inode>(new struct fs::vfs::inode);
    de->inode->direntries[0]->inode->type = fs::vfs::inode::type::REGULAR_FILE;
    de->inode->direntries.push_back(std::shared_ptr<struct fs::vfs::dirent>(new struct fs::vfs::dirent));
    de->inode->direntries[1]->name = "bash";
    de->inode->direntries[1]->inode = std::shared_ptr<struct fs::vfs::inode>(new struct fs::vfs::inode);
    de->inode->direntries[1]->inode->type = fs::vfs::inode::type::REGULAR_FILE;
    root->direntries.push_back(std::shared_ptr<struct fs::vfs::dirent>(de));

    de = new struct fs::vfs::dirent;
    de->name = "etc";
    de->inode = std::shared_ptr<struct fs::vfs::inode>(new struct fs::vfs::inode);
    de->inode->type = fs::vfs::inode::type::DIRECTORY;
    de->inode->direntries.push_back(std::shared_ptr<struct fs::vfs::dirent>(new struct fs::vfs::dirent));
    de->inode->direntries[0]->name = "test";
    de->inode->direntries[0]->inode = std::shared_ptr<struct fs::vfs::inode>(new struct fs::vfs::inode);
    de->inode->direntries[0]->inode->type = fs::vfs::inode::type::DIRECTORY;
    de->inode->direntries[0]->inode->direntries.push_back(std::shared_ptr<struct fs::vfs::dirent>(new struct fs::vfs::dirent));
    de->inode->direntries[0]->inode->direntries[0]->name = "test.conf";
    de->inode->direntries[0]->inode->direntries[0]->inode = std::shared_ptr<struct fs::vfs::inode>(new struct fs::vfs::inode);
    de->inode->direntries[0]->inode->direntries[0]->inode->type = fs::vfs::inode::type::REGULAR_FILE;
    de->inode->direntries.push_back(std::shared_ptr<struct fs::vfs::dirent>(new struct fs::vfs::dirent));
    de->inode->direntries[1]->name = "another-test";
    de->inode->direntries[1]->inode = std::shared_ptr<struct fs::vfs::inode>(new struct fs::vfs::inode);
    de->inode->direntries[1]->inode->type = fs::vfs::inode::type::DIRECTORY;
    de->inode->direntries[1]->inode->direntries.push_back(std::shared_ptr<struct fs::vfs::dirent>(new struct fs::vfs::dirent));
    de->inode->direntries[1]->inode->direntries[0]->name = "another_test.conf";
    de->inode->direntries[1]->inode->direntries[0]->inode = std::shared_ptr<struct fs::vfs::inode>(new struct fs::vfs::inode);
    de->inode->direntries[1]->inode->direntries[0]->inode->type = fs::vfs::inode::type::REGULAR_FILE;
    de->inode->direntries[1]->inode->direntries.push_back(std::shared_ptr<struct fs::vfs::dirent>(new struct fs::vfs::dirent));
    de->inode->direntries[1]->inode->direntries[1]->name = "another_test2.conf";
    de->inode->direntries[1]->inode->direntries[1]->inode = std::shared_ptr<struct fs::vfs::inode>(new struct fs::vfs::inode);
    de->inode->direntries[1]->inode->direntries[1]->inode->type = fs::vfs::inode::type::REGULAR_FILE;
    root->direntries.push_back(std::shared_ptr<struct fs::vfs::dirent>(de));

#endif // TEST_NEW_VFS
}

#ifdef TEST_NEW_VFS
static void print_node(std::shared_ptr<struct fs::vfs::inode> node, size_t depth) {
    std::string spaces;
    spaces.resize(depth * 4);
    for (size_t i = 0; i < depth * 4; i++) {
        spaces[i] = ' ';
    }

    if (node->type == fs::vfs::inode::type::DIRECTORY) {
        for (size_t i = 0; i < node->direntries.size(); i++) {
            kprintf(KP_INFO, "%s-> VFS node \"%s\"\n", spaces.c_str(), node->direntries[i]->name.c_str());
            if (node->type == fs::vfs::inode::type::DIRECTORY) {
                print_node(node->direntries[i]->inode, depth + 1);
            }
        }
    }
}
#endif // TEST_NEW_VFS

void fs::vfs::print_tree() {
#ifdef TEST_NEW_VFS
    kprintf(KP_INFO, "VFS tree:\n");
    print_node(root, 0);
    kprintf(KP_INFO, "VFS tree done!\n");
#endif // TEST_NEW_VFS
}

static std::vector<struct fs::vfs::fsinfo> mountpoints;

void fs::vfs::mount_fs(struct fsinfo fs, std::string mountpoint) {
    fs.mount_path = fs::path::split_path(&mountpoint);
    mountpoints.push_back(fs);
}

fs::vfs::file *fs::vfs::fopen(std::string path) {
    std::vector<std::string> split_path = fs::path::split_path(&path);
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
    *fileptr = mm::kmalloc(filesize + 30000); // FIXME: +30000 because kernel module code is broken and i need to sleep
    fread(file, *fileptr, filesize);
    fclose(file);
    return true;
}
