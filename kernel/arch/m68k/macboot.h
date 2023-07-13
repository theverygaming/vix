#ifndef _MACBOOT_H
#define _MACBOOT_H

#ifndef MACBOOT_HAS_STDINT
#include <stdint.h>
#endif

#define MAGIC1     (0x66786F6573203A33)
#define MAGIC2(id) (0x6375746500000000 | (id))
#define MAGIC3     (0x666C756666792C2C)

#ifdef __cplusplus
extern "C" {
#endif

#define MACBOOT_KERNEL_HEADER_ID \
    { MAGIC1, MAGIC3 }
struct __attribute__((packed)) macboot_kernel_header {
    uint64_t id[2];
    uint32_t load_address; // physical address where kernel executable will be loaded
    uint32_t size;         // size in memory (including .bss) - NOT the size on disk
    void (*kmain)();
};

struct __attribute__((packed)) macboot_kmem_response {
    uint32_t base;
    uint32_t size;
};

#define MACBOOT_KMEM_REQUEST_ID \
    { MAGIC1, MAGIC2(1), MAGIC3 }
struct __attribute__((packed)) macboot_kmem_request {
    uint64_t id[3];
    struct macboot_kmem_response *response;
};

#define MACBOOT_MEMMAP_USABLE                 (1)
#define MACBOOT_MEMMAP_RESERVED               (2)
#define MACBOOT_MEMMAP_BOOTLOADER_RECLAIMABLE (3)
struct __attribute__((packed)) macboot_memmap_response {
    uint32_t base;
    uint32_t size;
    int type;
    struct macboot_memmap_response *next;
};

#define MACBOOT_MEMMAP_REQUEST_ID \
    { MAGIC1, MAGIC2(2), MAGIC3 }
struct __attribute__((packed)) macboot_memmap_request {
    uint64_t id[3];
    struct macboot_memmap_response *response;
};

struct __attribute__((packed)) macboot_framebuffer_response {
    uint32_t base;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
};

#define MACBOOT_FRAMEBUFFER_REQUEST_ID \
    { MAGIC1, MAGIC2(3), MAGIC3 }
struct __attribute__((packed)) macboot_framebuffer_request {
    uint64_t id[3];
    struct macboot_framebuffer_response *response;
};

struct __attribute__((packed)) macboot_cmdline_response {
    const char *cmdline;
};

#define MACBOOT_CMDLINE_REQUEST_ID \
    { MAGIC1, MAGIC2(4), MAGIC3 }
struct __attribute__((packed)) macboot_cmdline_request {
    uint64_t id[3];
    struct macboot_cmdline_response *response;
};

struct __attribute__((packed)) macboot_terminal_response {
    void (*puts)(const char *str);
};

#define MACBOOT_TERMINAL_REQUEST_ID \
    { MAGIC1, MAGIC2(5), MAGIC3 }
struct __attribute__((packed)) macboot_terminal_request {
    uint64_t id[3];
    struct macboot_terminal_response *response;
};

#ifdef __cplusplus
}
#endif

#endif /* _MACBOOT_H */
