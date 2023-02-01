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
#include <arch/syscall_32.h>
#include <config.h>
#include <cppstd/string.h>
#include <cppstd/vector.h>
#include <framebuffer.h>
#include <fs/roramfs.h>
#include <fs/vfs.h>
#include <kernel.h>
#include <macros.h>
#include <mm/phys.h>
#include <panic.h>
#include <stdio.h>
#include <time.h>

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static void fbputc(char c) {
    fbconsole.putc(c);
}

static void *initramfs_start = nullptr;
static size_t initramfs_size = 0;

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
    if (multiboot2::find_initramfs(multiboot2_info_ptr, &initramfs_start, &initramfs_size)) {
        initramfs_size = ALIGN_UP(initramfs_size, 4096);
        printf("initramfs @ 0x%p size: 0x%p\n", initramfs_start, initramfs_size);
        if (!PTR_IS_ALIGNED(initramfs_start, ARCH_PAGE_SIZE)) {
            initramfs_size = 0;
        }
    }
    framebuffer.init(multiboot2::findFrameBuffer(multiboot2_info_ptr));
    fbconsole.init(&framebuffer);
    gdt::init();
    paging::init();
    kernelstart();
}

extern "C" uint8_t _bss_start;
extern "C" uint8_t _bss_end;

extern "C" void __attribute__((section(".entry"))) _kentry(void *multiboot2_info_ptr) {
    size_t sp;
    asm volatile("mov %%esp, %0" : "=a"(sp) :);
    if (sp < KERNEL_VIRT_ADDRESS) {
        return;
    }
    for (uint8_t *addr = &_bss_start; addr < &_bss_end; addr++) {
        *addr = 0;
    }
    kernelinit(multiboot2_info_ptr);
    while (true) {}
}

void arch::generic::startup::stage2_startup() {
    if (initramfs_size != 0) {
        mm::phys::phys_alloc(initramfs_start, initramfs_size / ARCH_PAGE_SIZE);
        paging::map_page(initramfs_start, (void *)(0xFFFFF000 - initramfs_size), (initramfs_size / ARCH_PAGE_SIZE), false, true);
    }
}

void arch::generic::startup::stage3_startup() {
    cpubasics::cpuinit(); // interrupt handlers are enabled here, before this all exceptions will cause a triplefault
    drivers::keyboard::init();
    // drivers::mouse::init(); // must be disabled when polling is in use for the keyboard
    isr::RegisterHandler(0x80, syscall::syscallHandler);
    cpuid::printFeatures();
    simd::enableSSE();
    if (initramfs_size != 0) {
        fs::filesystems::roramfs::init((void *)(0xFFFFF000 - initramfs_size));
        fs::filesystems::roramfs::mountInVFS();
    }
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
