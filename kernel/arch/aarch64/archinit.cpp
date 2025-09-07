#include <vix/arch/common/bootup.h>
#include <vix/arch/drivers/gpu/fb.h>
#include <vix/arch/drivers/timer.h>
#include <vix/arch/drivers/uart.h>
#include <vix/arch/limine.h>
#include <vix/arch/startup.h>
#include <vix/config.h>
#include <vix/framebuffer.h>
#include <vix/fs/tarfs.h>
#include <vix/fs/vfs.h>
#include <vix/kernel.h>
#include <vix/mm/kheap.h>
#include <vix/mm/memmap.h>
#include <vix/panic.h>
#include <vix/stdio.h>
#include <vix/time.h>

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static volatile uint8_t static_alloc_mem[4096 * 1000];

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST, .revision = 0
};
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0
};
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST, .revision = 0
};

static void fbputc(char c) {
    fbconsole.fbputc(c);
}

static volatile bool done = false;
static void cpuinit(uint64_t cpu) {
    while (done) {}
    printf("i am CPU %u\n", cpu);
    done = true;
    while (true) {}
}

// FIXME: whenever stack smashing protector is enabled on aarch64 it breaks

static void kernelinit() {
    if (framebuffer_request.response == nullptr ||
        framebuffer_request.response->framebuffer_count < 1) {
        while (true) {}
    }
    struct limine_framebuffer *limine_framebuffer =
        framebuffer_request.response->framebuffers[0];
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
    // puts("enabling timer\n");
    // drivers::timer::init();
    puts("kernelstart()\n");

    struct mm::mem_map_entry r;
    r.type = mm::mem_map_entry::type_t::RAM;
    r.base = (uintptr_t)static_alloc_mem;
    r.size = sizeof(static_alloc_mem) / sizeof(static_alloc_mem[0]);
    mm::set_mem_map(&r, 1);

    kernelstart();
}

extern "C" void _kentry() {
    kernelinit();
    while (true) {}
}

void arch::startup::stage2_startup() {}

void arch::startup::stage3_startup() {
    if (module_request.response != nullptr &&
        module_request.response->module_count > 0) {
        if (fs::filesystems::tarfs::init(
                module_request.response->modules[0]->address
            )) {
            fs::filesystems::tarfs::mountInVFS();
        }
    }
    framebuffer.clear();
    fbconsole.init2();
    stdio::set_putc_function(fbputc);
    printf("Hello aarch64!\n");
    time::bootupTime = time::getCurrentUnixTime();
}

void arch::startup::kthread0() {}
