#include <arch/x86/archinit.h>
#include <arch/x86/cpubasics.h>
#include <arch/x86/cpuid.h>
#include <arch/x86/drivers/keyboard.h>
#include <arch/x86/drivers/serial.h>
#include <arch/x86/drivers/text80x25.h>
#include <arch/x86/elf.h>
#include <arch/x86/gdt.h>
#include <arch/x86/generic/memory.h>
#include <arch/x86/generic/startup.h>
#include <arch/x86/isr.h>
#include <arch/x86/memorymap.h>
#include <arch/x86/modelf.h>
#include <arch/x86/multiboot2.h>
#include <arch/x86/multitasking.h>
#include <arch/x86/paging.h>
#include <arch/x86/simd.h>
#include <arch/x86/syscall.h>
#include <config.h>
#include <cppstd/string.h>
#include <cppstd/vector.h>
#include <framebuffer.h>
#include <fs/roramfs.h>
#include <fs/vfs.h>
#include <kernel.h>
#include <panic.h>
#include <stdio.h>
#include <time.h>

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static void idkputc(char c) {
    fbconsole.putc(c);
}

static void kernelinit(void *multiboot2_info_ptr) {
    drivers::textmode::text80x25::init();
    stdio::set_putc_function(drivers::textmode::text80x25::putc);
    drivers::serial::init();
    stdio::set_putc_function(drivers::serial::putc, true);
    if ((size_t)multiboot2_info_ptr & 7) {
        KERNEL_PANIC("multiboot2 info structure is not aligned, something is wrong here");
    }
    int memMap_count = 0;
    void *memMap = multiboot2::findMemMap(multiboot2_info_ptr, &memMap_count);
    memorymap::initMemoryMap(memMap, memMap_count);
    framebuffer.init(multiboot2::findFrameBuffer(multiboot2_info_ptr));
    for (size_t y = 0; y < framebuffer.get_height(); y++) {
        for (size_t x = 0; x < framebuffer.get_width(); x++) {
            framebuffer.write_pixel(x, y, 56, 60, 60); // discord background color
        }
    }
    fbconsole.init(&framebuffer);
    gdt::init();
    paging::clearPageTables((void *)0x0, KERNEL_VIRT_ADDRESS / ARCH_PAGE_SIZE);
    kernelstart();
}

extern "C" void __attribute__((section(".entry"))) _kentry(void *multiboot2_info_ptr) {
    size_t sp;
    asm volatile("mov %%esp, %0" : "=a"(sp) :);
    if (sp < KERNEL_VIRT_ADDRESS) {
        return;
    }
    kernelinit(multiboot2_info_ptr);
    while (true) {}
}

void arch::generic::startup::stage2_startup() {
    memalloc::page::kernel_alloc((void *)(KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET), 704);
}

void arch::generic::startup::stage3_startup() {
    cpubasics::cpuinit(); // interrupt handlers are enable here, before this all exceptions will cause a triplefault
    drivers::keyboard::init();
    isr::RegisterHandler(0x80, syscall::syscallHandler);
    cpuid::printFeatures();
    simd::enableSSE();
    fs::filesystems::roramfs::init((void *)(KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET));
    fs::filesystems::roramfs::mountInVFS();
    time::bootupTime = time::getCurrentUnixTime();
}

// very important array definitely
static uint8_t franxxlogo[9][18] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 4, 0},
    {0, 0, 0, 0, 0, 0, 0, 15, 0, 15, 0, 0, 0, 4, 0, 4, 0, 0},
    {0, 0, 1, 0, 1, 0, 1, 0, 15, 0, 4, 0, 4, 0, 4, 0, 0, 0},
    {0, 0, 0, 1, 0, 1, 0, 1, 0, 4, 0, 4, 0, 4, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 4, 0, 4, 0, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 1, 0, 1, 0, 4, 0, 4, 0, 4, 0, 0, 0, 0},
    {0, 0, 1, 0, 1, 0, 1, 0, 15, 0, 4, 0, 4, 0, 4, 0, 0, 0},
    {0, 1, 0, 1, 0, 0, 0, 15, 0, 15, 0, 0, 0, 4, 0, 4, 0, 0},
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

void arch::generic::startup::after_init() {
    fbconsole.init2();
    stdio::set_putc_function(idkputc);
    
    void *elfptr = nullptr;

    if (fs::vfs::fptr("/ramfs/module.o", &elfptr)) {
        printf("loading kernel module\n");
        elf::load_module(elfptr);
    }

    std::vector<std::string> args;
    if (fs::vfs::fptr("/ramfs/shitshell", &elfptr)) {
        args.push_back("/ramfs/shitshell");
        elf::load_program(elfptr, &args);
    }

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 18; j++) {
            putcolor(j + 63, i, franxxlogo[i][j] << 4 | 7);
        }
    }

    // a bit of a hack.. we have to call the vector destructor before killing this process
    args.~vector();

    multitasking::initMultitasking(); // this will kill this process
    while (true) {}
}
