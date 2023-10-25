#include <arch/common/bootup.h>
#include <arch/common/cpu.h>
#include <arch/drivers/serial.h>
#include <arch/limine.h>
#include <arch/paging.h>
#include <config.h>
#include <framebuffer.h>
#include <kernel.h>
#include <mm/kmalloc.h>
#include <mm/memmap.h>
#include <panic.h>
#include <stdio.h>
#include <time.h>

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static volatile struct limine_memmap_request memmap_request = {.id = LIMINE_MEMMAP_REQUEST, .revision = 0};

static volatile struct limine_terminal_request terminal_request = {.id = LIMINE_TERMINAL_REQUEST, .revision = 0};

static void limineputc(char c) {
    struct limine_terminal *terminal = terminal_request.response->terminals[0];
    terminal_request.response->write(terminal, &c, 1);
}

static void kernelinit() {
    if (terminal_request.response == NULL || terminal_request.response->terminal_count < 1) {
        while (true) {}
    }
    drivers::serial::init();
    stdio::set_putc_function(drivers::serial::putc, true);
    stdio::set_putc_function(limineputc, false);
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
    // stdio::set_putc_function(fbputc);
    printf("Hello x86_64!\n");
    time::bootupTime = time::getCurrentUnixTime();
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
