#include <arch/archinit.h>
#include <arch/cpubasics.h>
#include <arch/cpuid.h>
#include <arch/drivers/ps2.h>
#include <arch/drivers/serial.h>
#include <arch/drivers/text80x25.h>
#include <arch/elf.h>
#include <arch/gdt.h>
#include <arch/generic/memory.h>
#include <arch/generic/startup.h>
#include <arch/isr.h>
#include <arch/memorymap.h>
#include <arch/modelf.h>
#include <arch/multiboot2.h>
#include <arch/multitasking.h>
#include <arch/paging.h>
#include <arch/simd.h>
#include <arch/syscall.h>
#include <config.h>
#include <cppstd/string.h>
#include <cppstd/vector.h>
#include <framebuffer.h>
#include <fs/roramfs.h>
#include <fs/vfs.h>
#include <kernel.h>
#include <mm/phys.h>
#include <panic.h>
#include <stdio.h>
#include <time.h>

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static void fbputc(char c) {
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
    mm::phys::kernel_alloc((void *)(KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET), 704);
}

void arch::generic::startup::stage3_startup() {
    cpubasics::cpuinit(); // interrupt handlers are enabled here, before this all exceptions will cause a triplefault
    drivers::keyboard::init();
    // drivers::mouse::init(); // must be disabled when polling is in use for the keyboard
    isr::RegisterHandler(0x80, syscall::syscallHandler);
    cpuid::printFeatures();
    simd::enableSSE();
    fs::filesystems::roramfs::init((void *)(KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET));
    fs::filesystems::roramfs::mountInVFS();
    time::bootupTime = time::getCurrentUnixTime();
    framebuffer.clear();
    fbconsole.init2();
    stdio::set_putc_function(fbputc);
}

void arch::generic::startup::after_init() {
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

    // a bit of a hack.. we have to call the vector destructor before killing this process
    args.~vector();

    multitasking::initMultitasking(); // this will kill this process
    while (true) {}
}
