#include <arch/generic/memory.h>
#include <arch/generic/startup.h>
#include <config.h>
#include <framebuffer.h>
#include <kernel.h>
#include <kprintf.h>
#include <macros.h>
#include <mm/kmalloc.h>
#include <mm/memmap.h>
#include <mm/memtest.h>
#include <mm/phys.h>
#include <panic.h>
#include <stdio.h>
#include <time.h>
#define MACBOOT_HAS_STDINT
#include "macboot.h"

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static void fbputc(char c) {
    fbconsole.fbputc(c);
}

extern "C" void _kentry();
extern "C" void *KERNELMEMORY;
extern "C" void *__bss_size;
static volatile struct macboot_kernel_header __attribute__((section(".entry")))
header = {.id = MACBOOT_KERNEL_HEADER_ID, .load_address = 0x13000, .size = (uint32_t)&KERNELMEMORY, .kmain = &_kentry};

static volatile struct macboot_framebuffer_request fbreq = {.id = MACBOOT_FRAMEBUFFER_REQUEST_ID, .response = nullptr};
static volatile struct macboot_memmap_request memmapreq = {.id = MACBOOT_MEMMAP_REQUEST_ID, .response = nullptr};
static volatile struct macboot_kmem_request kmemreq = {.id = MACBOOT_KMEM_REQUEST_ID, .response = nullptr};

static void kernelinit() {
    if (fbreq.response != nullptr) {
        struct fb::fbinfo info = {
            .address = (void *)fbreq.response->base,
            .width = fbreq.response->width,
            .height = fbreq.response->height,
            .pitch = fbreq.response->pitch,
            .bpp = fbreq.response->bpp,
            .rgb = false,
            .monochrome = true,
        };

        framebuffer.init(info);
        fbconsole.init(&framebuffer);
        fbconsole.init2();
        stdio::set_putc_function(fbputc, true);
    }

    if (memmapreq.response != nullptr) {
        size_t entrycount = 0;
        struct macboot_memmap_response *entry = memmapreq.response;
        while (entry != nullptr) {
            entrycount++;
            entry = entry->next;
        }
        mm::set_mem_map(
            [](size_t n) -> struct mm::mem_map_entry {
                struct mm::mem_map_entry r;
                struct macboot_memmap_response *entry = memmapreq.response;
                for (size_t i = 0; i <= n; i++) {
                    if (i == n) {
                        r.base = entry->base;
                        r.size = entry->size;
                        switch (entry->type) {
                        case MACBOOT_MEMMAP_USABLE:
                            r.type = mm::mem_map_entry::type_t::RAM;
                            break;
                        case MACBOOT_MEMMAP_BOOTLOADER_RECLAIMABLE:
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
    }

    kernelstart();
}

extern "C" void _vec4();

extern "C" void vec4_cpp(uint32_t pc, uint16_t sr) {
    kprintf(KP_INFO, "invalid opcode: SR: 0x%p PC: 0x%p\n", (uint32_t)sr, pc);
    KERNEL_PANIC("exception");
}

extern "C" void _kentry() {
    uintptr_t *i_inst = (uintptr_t *)0x10;
    *i_inst = (uintptr_t)&_vec4;
    kernelinit();
    while (true) {}
}

void arch::generic::startup::stage2_startup() {
    if (kmemreq.response != nullptr) {
        mm::phys::phys_alloc((void *)kmemreq.response->base, ALIGN_UP(kmemreq.response->size, ARCH_PAGE_SIZE) / ARCH_PAGE_SIZE);
        kprintf(KP_INFO,
                "claimed 0x%p-0x%p as kernel memory\n",
                kmemreq.response->base,
                kmemreq.response->base + ALIGN_UP(kmemreq.response->size, ARCH_PAGE_SIZE));
    }
}

void arch::generic::startup::stage3_startup() {
    time::bootupTime = time::getCurrentUnixTime();
}

void arch::generic::startup::after_init() {
    while (true) {}
}
