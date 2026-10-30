#include <vix/arch/common/bootup.h>
#include <vix/arch/generic/memory.h>
#include <vix/config.h>
#include <vix/framebuffer.h>
#include <vix/interrupts.h>
#include <vix/kernel.h>
#include <vix/kprintf.h>
#include <vix/macros.h>
#include <vix/mm/kheap.h>
#include <vix/mm/memmap.h>
#include <vix/mm/memtest.h>
#include <vix/mm/pmm.h>
#include <vix/panic.h>
#include <vix/sched.h>
#include <vix/stdio.h>
#include <vix/time.h>
#include <vix/debug.h>
#include <vix/fs/tarfs.h>
#include <vix/abi/vix/vix.h>
#include <vector>
#include <string>
#include <vix/arch/trap.h>
#define LITTLEBOOT_HAS_STDINT
#include <littleboot/proto.h>

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static void fbputs(const char *str, size_t n) {
    fbconsole.fbputs(str, n);
}

extern "C" void __kentry();

extern "C" uint8_t KERNEL_START;
extern "C" uint8_t KERNEL_END;

static volatile struct littleboot_kernel_header __attribute__((section(".entry"))) header = {.id = LITTLEBOOT_KERNEL_HEADER_ID, .load_address = (uintptr_t)&KERNEL_START, .load_end_address = (uintptr_t)&KERNEL_END, .kmain = &__kentry};
static volatile struct littleboot_memmap_request memmapreq = {.id = LITTLEBOOT_MEMMAP_REQUEST_ID, .response = nullptr};
static volatile struct littleboot_kmem_request kmemreq = {.id = LITTLEBOOT_KMEM_REQUEST_ID, .response = nullptr};
static volatile struct littleboot_module_request modreq = {.id = LITTLEBOOT_MODULE_REQUEST_ID, .response = nullptr};

static void mmio_putc(char c) {
    *((volatile uint32_t *)0xF004B000) = c;
}

static void mmio_puts(const char *str, size_t n) {
    while (n) {
        mmio_putc(*(str++));
        n--;
    }
}

static void kernelinit() {
    mmio_puts("hi!\n", 4);
    stdio::set_puts_function(mmio_puts, true);
    if (memmapreq.response != nullptr) {
        size_t entrycount = 0;
        struct littleboot_memmap_response *entry = memmapreq.response;
        while (entry != nullptr) {
            entrycount++;
            entry = entry->next;
        }
        mm::set_mem_map(
            [](void *, size_t n) -> struct mm::mem_map_entry {
                struct mm::mem_map_entry r;
                struct littleboot_memmap_response *entry = memmapreq.response;
                for (size_t i = 0; i <= n; i++) {
                    if (i == n) {
                        r.base = entry->base;
                        r.size = entry->size;
                        switch (entry->type) {
                        case LITTLEBOOT_MEMMAP_USABLE:
                            r.type = mm::mem_map_entry::type_t::RAM;
                            break;
                        case LITTLEBOOT_MEMMAP_BOOTLOADER_RECLAIMABLE:
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

    trap_setup();

    kernelstart();
}

extern "C" void _kentry() {
    kernelinit();
    while (true) {}
}

void arch::startup::stage2_startup() {}

void arch::startup::stage3_startup() {
    if (kmemreq.response != nullptr) {
        mm::pmm::force_alloc_contiguous(ALIGN_DOWN(kmemreq.response->base, CONFIG_ARCH_PAGE_SIZE),
                                        ALIGN_UP(kmemreq.response->size, CONFIG_ARCH_PAGE_SIZE) / CONFIG_ARCH_PAGE_SIZE);
        kprintf(KP_INFO,
                "claimed 0x%p-0x%p as kernel memory\n",
                ALIGN_DOWN(kmemreq.response->base, CONFIG_ARCH_PAGE_SIZE),
                ALIGN_DOWN(kmemreq.response->base, CONFIG_ARCH_PAGE_SIZE) + ALIGN_UP(kmemreq.response->size, CONFIG_ARCH_PAGE_SIZE));
    }
}

void arch::startup::stage4_startup() {
    if (modreq.response != nullptr) {
        DEBUG_PRINTF("module at 0x%p size %u\n", modreq.response->address, modreq.response->size);
        // NOTE: we ignore all other modules if there's more than one.
        if (fs::filesystems::tarfs::init(
                modreq.response->address
            )) {
            fs::filesystems::tarfs::mountInVFS();
        }
    }
    time::bootupTime = time::getCurrentUnixTime();
    DEBUG_PRINTF("framebuffer init\n");
    struct fb::fbinfo fbinfo = {
        .address = (void *)0xF0000000,
        .width = 640,
        .height = 480,
        .pitch = 640 / 8,
        .bpp = 1,
        .rgb = true,
        .monochrome = true,
    };
    framebuffer.init(fbinfo);
    fbconsole.init(&framebuffer);
    stdio::set_puts_function(fbputs);
    framebuffer.clear();
    framebuffer.flush();
    fbconsole.init2();
    printf("hi!!\n");
}

static void kt(void *) {
    unsigned long counter = 0;
    while (true) {
        if (counter == 300000) {
            push_interrupt_disable();
            volatile int test = 5;
            kprintf(KP_INFO, "hi from kernel thread(TID %d) stack: 0x%p\n", sched::mythread()->tid, &test);
            pop_interrupt_disable();
            counter = 0;
        }
        counter++;
        sched::yield();
    }
}

void arch::startup::kthread0() {
    sched::start_kworker(kt);
    sched::start_kworker(kt);
    std::vector<std::string> args;
    args.push_back("/bin/sh");
    auto status = abi::vix::exec("/bin/sh", &args);
    if (!status.status().ok()) {
        kprintf(KP_WARNING, "could not load /bin/sh error: %d\n", status.status().code());
    }
}
