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

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static void fbputs(const char *str, size_t n) {
    fbconsole.fbputs(str, n);
}

extern "C" uint8_t __bss_start;
extern "C" uint8_t __bss_end;
extern "C" uint8_t __kernel_end;

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
    /*for (uint8_t *addr = &__bss_start; addr < &__bss_end; addr++) {
        *addr = 0;
    }*/
    stdio::set_puts_function(mmio_puts, true);
    mm::set_mem_map(
        [](void *, size_t n) -> struct mm::mem_map_entry {
            struct mm::mem_map_entry r;
            r.base = 0x00100000; // stack top
            r.size = 4096 * 19;
            r.type = mm::mem_map_entry::type_t::RAM;
            return r;
        },
        1);

    kernelstart();
}

extern "C" void _kentry() {
    kernelinit();
    while (true) {}
}

void arch::startup::stage2_startup() {}

void arch::startup::stage3_startup() {}

void arch::startup::stage4_startup() {
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
    while (true) {
        push_interrupt_disable();
        volatile int test = 5;
        kprintf(KP_INFO, "hi from kernel thread(TID %d) stack: 0x%p\n", sched::mythread()->tid, &test);
        pop_interrupt_disable();
        sched::yield();
    }
}

void arch::startup::kthread0() {
    sched::start_kworker(kt);
    sched::start_kworker(kt);
}

// FIXME: these are here because linking with libgcc is currently broken

extern "C" uint64_t __muldi3(uint64_t a, uint64_t b) {
    volatile uint64_t acc = 0;
    while (a--) {
        acc += b;
    }
    return acc;
}

extern "C" uint32_t __mulsi3(uint32_t a, uint32_t b) {
    volatile uint32_t acc = 0;
    while (a--) {
        acc += b;
    }
    return acc;
}

extern "C" uint32_t __umodsi3(uint32_t a, uint32_t b) {
    if (b == 0) { // bro really tryin to divide by zero :skull:
        return 0;
    }
    uint32_t r = a;
    uint32_t q = 0;
    while (r >= b) {
        r -= b;
        q++;
    }
    return r;
}

extern "C" uint32_t __udivsi3(uint32_t a, uint32_t b) {
    if (b == 0) { // bro really tryin to divide by zero :skull:
        return 0;
    }
    uint32_t r = a;
    uint32_t q = 0;
    while (r >= b) {
        r -= b;
        q++;
    }
    return q;
}

extern "C" int32_t __modsi3(int32_t a, int32_t b) {
    if (b == 0) { // bro really tryin to divide by zero :skull:
        return 0;
    }
    int32_t r = a;
    int32_t q = 0;
    while (r >= b) {
        r -= b;
        q++;
    }
    return q;
}
