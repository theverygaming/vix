#include <vix/arch/common/bootup.h>
#include <vix/arch/generic/memory.h>
#include <vix/config.h>
#include <vix/kernel.h>
#include <vix/kprintf.h>
#include <vix/macros.h>
#include <vix/mm/kheap.h>
#include <vix/mm/memmap.h>
#include <vix/mm/memtest.h>
#include <vix/mm/pmm.h>
#include <vix/panic.h>
#include <vix/stdio.h>
#include <vix/time.h>
#include <vix/sched.h>
#include <vix/interrupts.h>
#ifdef CONFIG_XTENSA_PLATFORM_ESP8266
#include <vix/arch/vectors.h>
#endif


#ifdef CONFIG_XTENSA_PLATFORM_ESP32
// ESP32 ROM
void (*ets_write_char_uart)(char c) = (void (*)(char))0x40007cf8;
#endif
#ifdef CONFIG_XTENSA_PLATFORM_ESP8266
// ESP8266 ROM
void (*ets_putc)(char c) = (void (*)(char))0x40002be8;
void (*ets_uart_printf)(const char *fmt, ...) = (void (*)(const char *, ...))0x40002544;
void (*Cache_Read_Enable)(uint8_t, uint8_t, uint8_t) = (void (*)(uint8_t, uint8_t, uint8_t))0x40004678;
#endif

static void romputs(const char *str, size_t n) {
    while (n) {
        char c = *(str++);
#ifdef CONFIG_XTENSA_PLATFORM_ESP32
        ets_write_char_uart(c);
#endif
#ifdef CONFIG_XTENSA_PLATFORM_ESP8266
        if (c == '\n') {
            ets_putc('\r');
        }
        ets_putc(c);
#endif
        n--;
    }
}

extern "C" uint8_t __bss_start;
extern "C" uint8_t __bss_end;

static void kernelinit() {
#ifdef CONFIG_XTENSA_PLATFORM_ESP8266
    uint32_t ps;
    asm volatile("rsr.ps %0" : "=a"(ps));
    ps |= 0xF; // set interrupt level to 15
    ps &= ~(1 << 5); // unset user mode bit
    asm volatile("wsr.ps %0" : : "a"(ps));
#endif
    stdio::set_puts_function(romputs, true);
    mm::set_mem_map(
        [](void *, size_t n) -> struct mm::mem_map_entry {
            struct mm::mem_map_entry r;

#ifdef CONFIG_XTENSA_PLATFORM_ESP32
            r.base = (uintptr_t)&__bss_end;
            r.size = (0x3FFF0000 + 0xFFFF) - (uintptr_t)&__bss_end;
            r.type = mm::mem_map_entry::type_t::RAM;
#endif
#ifdef CONFIG_XTENSA_PLATFORM_ESP8266
            r.base = (uintptr_t)&__bss_end;
            r.size = (0x3FFE8000 + 0x14000) - (uintptr_t)&__bss_end;
            r.type = mm::mem_map_entry::type_t::RAM;
#endif

            return r;
        },
        1);
#ifdef CONFIG_XTENSA_PLATFORM_ESP8266
    arch::init_vectors();
#endif
    kernelstart();
}

inline void write_addr_32(uint32_t *addr, uint32_t value) {
    volatile uint32_t *ptr = (volatile uint32_t *)addr;
    *ptr = value;
}

extern "C" void __attribute__((section(".entry"))) _kentry() {
    for (uint8_t *addr = &__bss_start; addr < &__bss_end; addr++) {
        *addr = 0;
    }

#ifdef CONFIG_XTENSA_PLATFORM_ESP8266
    // huge thanks to https://richard.burtons.org/2015/06/12/esp8266-cache_read_enable/
    Cache_Read_Enable(0, 0, 1); // map first 1MiB of flash
#endif

#ifdef CONFIG_XTENSA_PLATFORM_ESP32
    // HACK: disable the WDT's
    // RTC WDT
    write_addr_32((uint32_t *)0x3FF480A4, 0x050D83AA1); // RTC_CNTL_WDTWPROTECT_REG
    write_addr_32((uint32_t *)0x3FF4808C, 0x00);        // RTC_CNTL_WDTCONFIG0_REG
    // system WDT
    /*
    write_addr_32((uint32_t *)0x3FF5F064, 0x050D83AA1); // TIMG0_T0_WDTWPROTECT_REG
    write_addr_32((uint32_t *)0x3FF60064, 0x050D83AA1); // TIMG1_T1_WDTWPROTECT_REG
    write_addr_32((uint32_t *)0x3FF5F048, 0x00);        // TIMG0_T0_WDTCONFIG0_REG
    write_addr_32((uint32_t *)0x3FF60048, 0x00);        // TIMG1_T1_WDTCONFIG0_REG
    */
#endif

    kernelinit();
    while (true) {}
}

void arch::startup::stage2_startup() {}

void arch::startup::stage3_startup() {}

void arch::startup::stage4_startup() {
    time::bootupTime = time::getCurrentUnixTime();
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
