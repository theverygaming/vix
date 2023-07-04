#include <arch/generic/startup.h>
#include <config.h>
#include <framebuffer.h>
#include <kernel.h>
#include <kprintf.h>
#include <macros.h>
#include <mm/kmalloc.h>
#include <mm/memmap.h>
#include <mm/memtest.h>
#include <panic.h>
#include <stdio.h>
#include <time.h>

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static void fbputc(char c) {
    fbconsole.fbputc(c);
}

#define BOOT_MAGIC (0x66786F6573203A33)

struct memmap_entry {
    uint32_t base;
    uint32_t size;
    enum class type { USABLE, RESERVED, BAD_MEMORY, BOOTLOADER_RECLAIMABLE, KERNEL, MMIO } type;
    struct memmap_entry *next;
};

struct boot_fbinfo {
    void *base;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
};

struct bootloaderinfo {
    struct memmap_entry *memmap_first;
    struct boot_fbinfo *fbinfo;
};

struct module {
    uint32_t disk_offset;
    uint32_t size; // no module present if zero
};

struct bootheader {
    uint64_t magic;
    uint32_t load_adr;              // physical address to load kernel at
    uint32_t size;                  // kernel size (including .bss)
    uint32_t disksize;              // kernel size on disk
    void (*kmain)(uint32_t modadr); // kernel init function (if modaddr argument is zero then no module present)
    struct module mod;
    struct bootloaderinfo info;
};

static void bootloaderputc(char c) {
    void (*scrnps)(const char *str) = (void (*)(const char *))0x10312;
    static char strx[] = {0, 0};
    strx[0] = c;
    scrnps(strx);
}

extern "C" void _kentry(uint32_t modadr);
extern "C" void *KERNELMEMORY;
extern "C" void *__bss_size;
struct bootheader __attribute__((section(".entry"))) header = {.magic = BOOT_MAGIC,
                                                               .load_adr = 0x11000,
                                                               .size = (uint32_t)&KERNELMEMORY,
                                                               .disksize = (uint32_t)&KERNELMEMORY,
                                                               .kmain = &_kentry,
                                                               .mod =
                                                                   {
                                                                       .disk_offset = 0,
                                                                       .size = 0,
                                                                   },
                                                               .info = {.memmap_first = nullptr, .fbinfo = nullptr}};

static void kernelinit() {
    stdio::set_putc_function(bootloaderputc, true);
    struct fb::fbinfo info = {
        .address = header.info.fbinfo->base,
        .width = header.info.fbinfo->width,
        .height = header.info.fbinfo->height,
        .pitch = header.info.fbinfo->pitch,
        .bpp = header.info.fbinfo->bpp,
        .rgb = false,
        .monochrome = true,
    };

    framebuffer.init(info);
    fbconsole.init(&framebuffer);
    fbconsole.init2();
    stdio::set_putc_function(fbputc, true);

    size_t entrycount = 0;
    struct memmap_entry *entry = header.info.memmap_first;
    while (entry) {
        if (entry->type != memmap_entry::type::KERNEL) {
            entrycount++;
        }
        entry = entry->next;
    }
    mm::set_mem_map(
        [](size_t n) -> struct mm::mem_map_entry {
            struct mm::mem_map_entry r;
            struct memmap_entry *entry = header.info.memmap_first;
            for (size_t i = 0; i <= n; i++) {
                if (entry->type == memmap_entry::type::KERNEL) {
                    entry = entry->next;
                    i -= 1;
                    continue;
                }
                if (i == n) {
                    r.base = entry->base;
                    r.size = entry->size;
                    switch (entry->type) {
                    case memmap_entry::type::USABLE:
                        r.type = mm::mem_map_entry::type_t::RAM;
                        break;
                    case memmap_entry::type::BOOTLOADER_RECLAIMABLE:
                        r.type = mm::mem_map_entry::type_t::RECLAIMABLE;
                        // r.type = mm::mem_map_entry::type_t::RAM; // we immediately reclaim the bootloader memory
                        break;
                    default:
                        r.type = mm::mem_map_entry::type_t::RESERVED;
                    }
                }
                entry = entry->next;
            }
            return r;
        },
        entrycount);

    kernelstart();
}

extern "C" void _kentry(uint32_t modadr) {
    kernelinit();
    while (true) {}
}

void arch::generic::startup::stage2_startup() {}

void arch::generic::startup::stage3_startup() {
    time::bootupTime = time::getCurrentUnixTime();
}

void arch::generic::startup::after_init() {
    while (true) {}
}
