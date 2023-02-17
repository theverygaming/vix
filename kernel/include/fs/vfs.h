#pragma once
#include <cppstd/string.h>
#include <cppstd/vector.h>
#include <mm/kmalloc.h>
#include <types.h>

#define VFS_SEEK_END (1 << 0)

namespace fs::vfs {
    /* new */
    struct __attribute__((packed)) fsinfo { // TODO: only packed because no aligned malloc
        void *info;                         // for internal use by the filesystem

        void *(*fopen)(void *info, std::vector<std::string> *path);
        void (*fclose)(void *info, void *file);

        size_t (*fread)(void *info, void *file, void *buf, size_t count);

        size_t (*ftell)(void *info, void *file);
        void (*fseek)(void *info, void *file, size_t pos, unsigned int flags);

        /* private to VFS */
        std::vector<std::string> mount_path;
    };

    typedef struct __attribute__((packed)) { // TODO: only packed because no aligned malloc
        struct fsinfo mount;                 // do not use directly, currently we do not keep track of all open files so on every operation we have to check if this is still valid
        void *internal_file;
    } file;

    void mount_fs(struct fsinfo fs, std::string mountpoint);

    file *fopen(std::string *path);
    void fclose(file *file);

    size_t fread(file *file, void *buf, size_t count);

    size_t ftell(file *file);
    void fseek(file *file, size_t pos, unsigned int flags = 0);

    /* old */
    bool fptr(const char *path, void **fileptr);
}
