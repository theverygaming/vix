#include "roramfs.h"
#include <fs/path.h>
#include <fs/vfs.h>
#include <stdlib.h>
#include <types.h>

#include <stdio.h>

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
    for (int i = 0; i < fsheader.filecount; i++) {
        if (fs::path::path_compare(fileptr->name, path)) {
            return fileptr->size;
        }
        fileptr++;
    }
    return 0;
}

static bool fload(char *path, void *memloc) {
    struct file_entry *fileptr = (struct file_entry *)(((uint8_t *)locationptr) + sizeof(struct header));
    for (int i = 0; i < fsheader.filecount; i++) {
        if (fs::path::path_compare(fileptr->name, path)) {
            memcpy((char *)memloc, (char *)(((uint8_t *)locationptr) + fileptr->offset), fileptr->size);
            return true;
        }
        fileptr++;
    }
    return false;
}

static bool fptr(char *path, void **fileptr) {
    struct file_entry *file_ptr = (struct file_entry *)(((uint8_t *)locationptr) + sizeof(struct header));
    for (int i = 0; i < fsheader.filecount; i++) {
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
    memcpy((char *)&fsheader, (char *)locationptr, sizeof(header));
    printf("loaded roramfs - name: %s -- files: %d\n", fsheader.name, fsheader.filecount);
}

void fs::filesystems::roramfs::deinit() {
    fs::vfs::unmount("/ramfs/");
}

void fs::filesystems::roramfs::mountInVFS() {
    struct fs::vfs::vfs_mountpoint mountpoint = {.uid = 0, .gid = 0, .fsize = &fsize, .fload = &fload, .fptr = &fptr};
    printf("filecount: %d\n", fsheader.filecount);
    strcpy(mountpoint.mountpath, "/ramfs/");
    fs::vfs::mount(mountpoint);
}