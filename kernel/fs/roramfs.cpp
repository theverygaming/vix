#include <fs/path.h>
#include <fs/roramfs.h>
#include <fs/vfs.h>
#include <log.h>
#include <stdlib.h>
#include <types.h>

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

static size_t fsize(char *path) {
    struct file_entry *fileptr = (struct file_entry *)(((uint8_t *)locationptr) + sizeof(struct header));
    for (uint32_t i = 0; i < fsheader.filecount; i++) {
        if (fs::path::path_compare(fileptr->name, path)) {
            return fileptr->size;
        }
        fileptr++;
    }
    return 0;
}

static bool fload(char *path, void *memloc) {
    struct file_entry *fileptr = (struct file_entry *)(((uint8_t *)locationptr) + sizeof(struct header));
    for (uint32_t i = 0; i < fsheader.filecount; i++) {
        if (fs::path::path_compare(fileptr->name, path)) {
            memcpy(memloc, ((uint8_t *)locationptr) + fileptr->offset, fileptr->size);
            return true;
        }
        fileptr++;
    }
    return false;
}

static bool fptr(char *path, void **fileptr) {
    struct file_entry *file_ptr = (struct file_entry *)(((uint8_t *)locationptr) + sizeof(struct header));
    for (uint32_t i = 0; i < fsheader.filecount; i++) {
        if (fs::path::path_compare(file_ptr->name, path)) {
            *fileptr = (((uint8_t *)locationptr) + file_ptr->offset);
            return true;
        }
        file_ptr++;
    }
    return false;
}

void fs::filesystems::roramfs::init(void *location) {
    locationptr = location;
    memcpy(&fsheader, locationptr, sizeof(header));
    DEBUG_PRINTF("loaded roramfs - name: %s -- files: %d\n", fsheader.name, fsheader.filecount);
    log::log_service("roramfs", "initialized");
}

void fs::filesystems::roramfs::deinit() {
    
}

void fs::filesystems::roramfs::mountInVFS() {
    log::log_service("roramfs", "mounted");
}
