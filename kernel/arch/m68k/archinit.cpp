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

#ifndef CONFIG_PLAIN_BINARY
#define MACBOOT_HAS_STDINT
#include "macboot.h"
static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static void fbputc(char c) {
    fbconsole.fbputc(c);
}

extern "C" void _kentry();
extern "C" void *PHYS_START;
extern "C" void *__kernel_size;
static volatile struct macboot_kernel_header __attribute__((section(".entry")))
header = {.id = MACBOOT_KERNEL_HEADER_ID, .load_address = (uint32_t)&PHYS_START, .size = (uint32_t)&__kernel_size, .kmain = &_kentry};

static volatile struct macboot_framebuffer_request fbreq = {.id = MACBOOT_FRAMEBUFFER_REQUEST_ID, .response = nullptr};
static volatile struct macboot_memmap_request memmapreq = {.id = MACBOOT_MEMMAP_REQUEST_ID, .response = nullptr};
static volatile struct macboot_kmem_request kmemreq = {.id = MACBOOT_KMEM_REQUEST_ID, .response = nullptr};
#else
extern "C" uint8_t __bss_start;
extern "C" uint8_t __bss_end;
extern "C" uint8_t __kernel_end;

static void mmio_putc(char c) {
    //*((volatile uint16_t *)0x100000) = (uint16_t)c;
    *((volatile uint32_t *)0x100000) = (uint32_t)c | ((uint32_t)c << 8) | ((uint32_t)c << 16) | ((uint32_t)c << 24);
    //for(int i = 0; i < 6942; i++) {}
}
#endif

static void kernelinit() {
#ifndef CONFIG_PLAIN_BINARY
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
#else
    mmio_putc('C');
    mmio_putc('P');
    mmio_putc('P');
    mmio_putc('\n');
    for (uint8_t *addr = &__bss_start; addr < &__bss_end; addr++) {
        *addr = 0;
    }
    stdio::set_putc_function(mmio_putc, true);

    mm::set_mem_map(
        [](size_t n) -> struct mm::mem_map_entry {
            struct mm::mem_map_entry r;
            r.base = (uintptr_t)&__kernel_end;
            r.size = (0xFFFC - 0x1000 /* stack.. */) - ((uintptr_t)&__kernel_end);
            r.type = mm::mem_map_entry::type_t::RAM;
            return r;
        },
        1);
#endif

    kernelstart();
}

extern "C" void _vectest();

extern "C" void vectest_cpp(uint32_t pc, uint16_t sr) {
    kprintf(KP_INFO, "invalid opcode: SR: 0x%p PC: 0x%p\n", (uint32_t)sr, pc);
    KERNEL_PANIC("exception");
}

extern "C" void _kentry() {
    //uint32_t *i_inst = (uint32_t *)(sizeof(uint32_t) * 31);
    //*i_inst = (uint32_t)&_vectest;
    //asm volatile("move.w #0x2400, %sr"); // supervisor set, Interrupt priority 6
    kernelinit();
    while (true) {}
}

void arch::startup::stage2_startup() {
#ifndef CONFIG_PLAIN_BINARY
    if (kmemreq.response != nullptr) {
        mm::pmm::force_alloc_contiguous((void *)ALIGN_DOWN(kmemreq.response->base, ARCH_PAGE_SIZE),
                                        ALIGN_UP(kmemreq.response->size, ARCH_PAGE_SIZE) / ARCH_PAGE_SIZE);
        kprintf(KP_INFO,
                "claimed 0x%p-0x%p as kernel memory\n",
                ALIGN_DOWN(kmemreq.response->base, ARCH_PAGE_SIZE),
                ALIGN_DOWN(kmemreq.response->base, ARCH_PAGE_SIZE) + ALIGN_UP(kmemreq.response->size, ARCH_PAGE_SIZE));
    }
#endif
}

void arch::startup::stage3_startup() {
    time::bootupTime = time::getCurrentUnixTime();
}

static void kt() {
    while (true) {
        push_interrupt_disable();
        volatile int test = 5;
        kprintf(KP_INFO, "hi from kernel thread(PID %d) stack: 0x%p\n", sched::mypid(), &test);
        pop_interrupt_disable();
        sched::yield();
    }
}

void arch::startup::kthread0() {
    sched::start_thread(kt);
    sched::start_thread(kt);
}
