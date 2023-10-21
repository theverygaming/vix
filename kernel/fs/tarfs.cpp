#include <debug.h>
#include <fs/path.h>
#include <fs/tarfs.h>
#include <fs/vfs.h>
#include <macros.h>
#include <stdlib.h>
#include <string>
#include <types.h>
#include <vector>

// https://www.gnu.org/software/tar/manual/html_node/Standard.html
struct __attribute__((packed)) tarheader {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
};

// all the tar stuff is based on https://opensource.apple.com/source/libarchive/libarchive-32/libarchive/contrib/untar.c.auto.html

static void *locationptr;

struct __attribute__((packed)) file {
    uint32_t position;
    size_t size;
    void *base;
};

static bool is_zero(const void *mem, size_t size) {
    const uint8_t *mem_p = (const uint8_t *)mem;
    for (size_t i = 0; i < size; i++) {
        if (mem_p[i] != 0) {
            return false;
        }
    }
    return true;
}

static size_t parse_octal(const char *p, size_t n) {
    size_t i = 0;

    while (*p < '0' || *p > '7') {
        ++p;
        --n;
    }
    while (*p >= '0' && *p <= '7' && n > 0) {
        i *= 8;
        i += *p - '0';
        ++p;
        --n;
    }
    return i;
}

static bool verify_checksum(const void *mem) {
    const uint8_t *ptr = (const uint8_t *)mem;
    int n, u = 0;
    for (n = 0; n < 512; ++n) {
        if (n < 148 || n > 155)
            /* Standard tar checksum adds unsigned bytes. */
            u += ptr[n];
        else
            u += 0x20;
    }
    return (u == parse_octal((const char *)ptr + 148, 8));
}

static void *fopen(void *info, std::vector<std::string> *path) {
    struct tarheader *file_ptr = (struct tarheader *)locationptr;
    while (!is_zero(file_ptr, 512 * 2)) {
        if (memcmp(file_ptr->magic, "ustar", 6)) {
            DEBUG_PRINTF("invalid magic\n");
            return nullptr;
        }

        if (!verify_checksum(file_ptr)) {
            DEBUG_PRINTF("tar checksum failure\n");
            return nullptr;
        }

        size_t size = parse_octal(file_ptr->size, sizeof(file_ptr->size));

        if (file_ptr->typeflag == '0') {
            std::string p = file_ptr->name;
            std::vector<std::string> p_s = fs::path::split_path(&p);
            if (fs::path::equals(path, &p_s)) {
                struct file *f = new struct file;
                f->position = 0;
                f->size = size;
                f->base = (uint8_t *)file_ptr + 512;
                return f;
            }
        }

        file_ptr = (struct tarheader *)(PTR_ALIGN_UP((uint8_t *)file_ptr + 512 + size, 512));
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
    if ((count + f->position) > f->size) {
        count = f->size - f->position;
    }

    memcpy(buf, (uint8_t *)f->base + f->position, count);

    f->position += count;

    return count;
}

static size_t ftell(void *info, void *file) {
    struct file *f = (struct file *)file;
    return f->position;
}

static void fseek(void *info, void *file, size_t pos, unsigned int flags) {
    struct file *f = (struct file *)file;

    if (pos >= f->size) {
        pos = f->size;
    }

    if (flags & VFS_SEEK_END) {
        pos = f->size;
    }

    f->position = pos;
}

bool fs::filesystems::tarfs::init(void *location) {
    locationptr = location;
    uint32_t total_size = 0;

    struct tarheader *file_ptr = (struct tarheader *)locationptr;
    while (!is_zero(file_ptr, 512 * 2)) {
        if (memcmp(file_ptr->magic, "ustar", 6)) {
            DEBUG_PRINTF("invalid magic\n");
            return false;
        }

        if (!verify_checksum(file_ptr)) {
            DEBUG_PRINTF("tar checksum failure\n");
            return false;
        }

        size_t size = parse_octal(file_ptr->size, sizeof(file_ptr->size));
        total_size += size;

        if (file_ptr->typeflag == '0') {
            DEBUG_PRINTF("    -> %s %u\n", file_ptr->name, size);
        } else {
            DEBUG_PRINTF("    -> %s %u [UNSUPPORTED]\n", file_ptr->name, size);
        }

        file_ptr = (struct tarheader *)(PTR_ALIGN_UP((uint8_t *)file_ptr + 512 + size, 512));
    }

    DEBUG_PRINTF("    -> total: %u\n", total_size);
    kprintf(KP_INFO, "tarfs: initialized\n");
    return true;
}

void fs::filesystems::tarfs::deinit() {}

void fs::filesystems::tarfs::mountInVFS() {
    struct fs::vfs::fsinfo fs {
        .info = nullptr, .fopen = fopen, .fclose = fclose, .fread = fread, .ftell = ftell, .fseek = fseek,
    };

    fs::vfs::mount_fs(fs, "/");

    kprintf(KP_INFO, "tarfs: mounted\n");
}
