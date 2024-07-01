#include <arch/common/bootup.h>
#include <arch/common/cpu.h>
#include <arch/drivers/serial.h>
#include <arch/limine.h>
#include <arch/paging.h>
#include <config.h>
#include <framebuffer.h>
#include <fs/tarfs.h>
#include <fs/vfs.h>
#include <kernel.h>
#include <mm/kheap.h>
#include <mm/memmap.h>
#include <panic.h>
#include <stdio.h>
#include <time.h>

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static volatile struct limine_memmap_request memmap_request = {.id = LIMINE_MEMMAP_REQUEST, .revision = 0};
static volatile struct limine_framebuffer_request framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};
static volatile struct limine_module_request module_request = {.id = LIMINE_MODULE_REQUEST, .revision = 0};

static void fbputc(char c) {
    fbconsole.fbputc(c);
}

static void kernelinit() {
    if (framebuffer_request.response == nullptr || framebuffer_request.response->framebuffer_count < 1) {
        while (true) {}
    }
    drivers::serial::init();
    stdio::set_putc_function(drivers::serial::putc, true);
    struct limine_framebuffer *limine_framebuffer = framebuffer_request.response->framebuffers[0];
    struct fb::fbinfo fbinfo = {
        .address = limine_framebuffer->address,
        .width = limine_framebuffer->width,
        .height = limine_framebuffer->height,
        .pitch = limine_framebuffer->pitch,
        .bpp = limine_framebuffer->bpp,
        .rgb = true,
        .monochrome = false,
    };
    framebuffer.init(fbinfo);
    fbconsole.init(&framebuffer);
    puts("entry\n");
    puts("initializing paging\n");
    paging::init();
    if (memmap_request.response != NULL) {
        mm::set_mem_map(
            [](size_t n) -> struct mm::mem_map_entry {
                struct mm::mem_map_entry r;

                r.base = memmap_request.response->entries[n]->base;
                r.size = memmap_request.response->entries[n]->length;
                switch (memmap_request.response->entries[n]->type) {
                case LIMINE_MEMMAP_USABLE:
                    r.type = mm::mem_map_entry::type_t::RAM;
                    break;
                case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
                    r.type = mm::mem_map_entry::type_t::ACPI_RECLAIM;
                    break;
                case LIMINE_MEMMAP_ACPI_NVS:
                    r.type = mm::mem_map_entry::type_t::ACPI_NVS;
                    break;
                case LIMINE_MEMMAP_BAD_MEMORY:
                    r.type = mm::mem_map_entry::type_t::UNUSABLE;
                    break;
                default:
                    r.type = mm::mem_map_entry::type_t::RESERVED;
                }

                return r;
            },
            memmap_request.response->entry_count);
    }

    puts("kernelstart()\n");
    kernelstart();
}

extern "C" void _kentry() {
    kernelinit();
    while (true) {}
}

void arch::startup::stage2_startup() {}

void arch::startup::stage3_startup() {
    time::bootupTime = time::getCurrentUnixTime();
    if (module_request.response != nullptr && module_request.response->module_count > 0) {
        if (fs::filesystems::tarfs::init(module_request.response->modules[0]->address)) {
            fs::filesystems::tarfs::mountInVFS();
        }
    }
    framebuffer.clear();
    fbconsole.init2();
    stdio::set_putc_function(fbputc);
    printf("Hello x86_64!\n");
}

void arch::startup::kthread0() {}

// dummy functions
#include <arch/drivers/text80x25.h>
#include <arch/idt.h>
#include <arch/isr.h>

void isr::RegisterHandler(int, void (*)(struct arch::full_ctx *)) {}
void drivers::textmode::text80x25::delc() {}
void drivers::textmode::text80x25::putc(char c) {}
void drivers::textmode::text80x25::putc(char c, color foreground, color background) {}
void idt::i686_IDT_Initialize() {}
void isr::i686_ISR_Initialize() {}
extern "C" void GDT_load_32() {}
extern "C" void simd_enable_sse() {}
extern "C" void x86_load_cpu_full_ctx() {}
