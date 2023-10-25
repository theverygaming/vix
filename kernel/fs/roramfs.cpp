#include <debug.h>
#include <fs/path.h>
#include <fs/roramfs.h>
#include <fs/vfs.h>
#include <kprintf.h>
#include <stdlib.h>
#include <string>
#include <types.h>
#include <vector>

/*
struct __attribute__((packed)) file_entry {
    char name[100];
    uint32_t size;
    uint32_t offset;
};

struct __attribute__((packed)) header {
    char name[100];
    uint32_t filecount;
};

static void *locationptr;

static struct header fsheader;

struct __attribute__((packed)) file {
    uint32_t position;
    struct file_entry *file_ptr;
};

static void *fopen(void *info, std::vector<std::string> *path) {
    struct file_entry *file_ptr = (struct file_entry *)(((uint8_t *)locationptr) + sizeof(struct header));
    for (uint32_t i = 0; i < fsheader.filecount; i++) {
        std::string p = file_ptr->name;
        std::vector<std::string> p_s = fs::path::split_path(p);
        if (fs::path::equals(path, &p_s)) {
            struct file *f = new struct file;
            f->position = 0;
            f->file_ptr = file_ptr;
            return f;
        }
        file_ptr++;
    }
    return nullptr;
}

static void fclose(void *info, void *file) {
    struct file *f = (struct file *)file;
    if (f == nullptr) {
        return;
    }
    delete f;
}

static size_t fread(void *info, void *file, void *buf, size_t count) {
    struct file *f = (struct file *)file;
    if ((count + f->position) > f->file_ptr->size) {
        count = f->file_ptr->size - f->position;
    }

    memcpy(buf, ((uint8_t *)locationptr) + f->file_ptr->offset + f->position, count);

    f->position += count;

    return count;
}

static size_t ftell(void *info, void *file) {
    struct file *f = (struct file *)file;
    return f->position;
}

static void fseek(void *info, void *file, size_t pos, unsigned int flags) {
    struct file *f = (struct file *)file;

    if (pos >= f->file_ptr->size) {
        pos = f->file_ptr->size;
    }

    if (flags & VFS_SEEK_END) {
        pos = f->file_ptr->size;
    }

    f->position = pos;
}

void fs::filesystems::roramfs::init(void *location) {
    locationptr = location;
    memcpy(&fsheader, locationptr, sizeof(header));
    DEBUG_PRINTF("loaded roramfs - name: %s -- files: %d\n", fsheader.name, fsheader.filecount);
    uint32_t total_size = 0;
    struct file_entry *file_ptr = (struct file_entry *)(((uint8_t *)locationptr) + sizeof(struct header));
    for (uint32_t i = 0; i < fsheader.filecount; i++) {
        DEBUG_PRINTF("    -> %s %u\n", file_ptr->name, file_ptr->size);
        total_size += file_ptr->size;
        file_ptr++;
    }
    DEBUG_PRINTF("    -> total: %u\n", total_size);
    kprintf(KP_INFO, "roramfs: initialized\n");
}

void fs::filesystems::roramfs::deinit() {}

void fs::filesystems::roramfs::mountInVFS() {
    struct fs::vfs::fsinfo fs {
        .info = nullptr, .fopen = fopen, .fclose = fclose, .fread = fread, .ftell = ftell, .fseek = fseek,
    };

    fs::vfs::mount_fs(fs, "/");

    kprintf(KP_INFO, "roramfs: mounted\n");
}
*/
