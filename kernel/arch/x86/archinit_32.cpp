#include <string>
#include <vector>
#include <vix/arch/archinit.h>
#include <vix/arch/common/bootup.h>
#include <vix/arch/common/cpu.h>
#include <vix/arch/cpubasics.h>
#include <vix/arch/cpuid.h>
#include <vix/arch/drivers/ps2.h>
#include <vix/arch/drivers/serial.h>
#include <vix/arch/drivers/text80x25.h>
#include <vix/arch/elf.h>
#include <vix/arch/gdt.h>
#include <vix/arch/generic/memory.h>
#include <vix/arch/isr.h>
#include <vix/arch/memorymap.h>
#include <vix/arch/modelf.h>
#include <vix/arch/multiboot2.h>
#include <vix/arch/multitasking.h>
#include <vix/arch/paging.h>
#include <vix/arch/simd.h>
#include <vix/arch/syscall_32.h>
#include <vix/arch/tss.h>
#include <vix/config.h>
#include <vix/framebuffer.h>
#include <vix/fs/tarfs.h>
#include <vix/fs/vfs.h>
#include <vix/interrupts.h>
#include <vix/kernel.h>
#include <vix/macros.h>
#include <vix/mm/pmm.h>
#include <vix/panic.h>
#include <vix/sched.h>
#include <vix/stdio.h>
#include <vix/time.h>

fb::fb framebuffer; // HACK: exported so modules can use it // TODO: have a central framebuffer manager that takes care of this
static fb::fbconsole fbconsole;

static void fbputc(char c) {
    fbconsole.fbputc(c);
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
        kprintf(KP_INFO, "archinit: initramfs @ 0x%p size: 0x%p\n", initramfs_start, initramfs_size);
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

void arch::startup::stage2_startup() {
    mm::pmm::force_alloc_contiguous((void *)KERNEL_PHYS_ADDRESS, ALIGN_UP(KERNEL_FREE_AREA_BEGIN_OFFSET, ARCH_PAGE_SIZE) / ARCH_PAGE_SIZE);
    if (initramfs_size != 0) {
        mm::pmm::force_alloc_contiguous(initramfs_start, ALIGN_UP(initramfs_size, ARCH_PAGE_SIZE) / ARCH_PAGE_SIZE);
        paging::map_page(initramfs_start,
                         (void *)(0xFFFFF000 - ALIGN_UP(initramfs_size, ARCH_PAGE_SIZE)),
                         (ALIGN_UP(initramfs_size, ARCH_PAGE_SIZE) / ARCH_PAGE_SIZE),
                         false,
                         true);
    }
}

void arch::startup::stage3_startup() {
    cpubasics::cpuinit(); // interrupt handlers are enabled here, before this all exceptions will cause a triplefault
    drivers::ps2_keyboard::init();
    drivers::mouse::init(); // must be disabled when polling is in use for the keyboard
    // isr::RegisterHandler(0x80, syscall::syscallHandler);
    cpuid::printFeatures();
    simd::enableSSE();
    if (initramfs_size != 0) {
        if (fs::filesystems::tarfs::init((void *)(0xFFFFF000 - initramfs_size))) {
            fs::filesystems::tarfs::mountInVFS();
        }
    }
    time::bootupTime = time::getCurrentUnixTime();
    framebuffer.clear();
    fbconsole.init2();
    stdio::set_putc_function(fbputc);
}

static void kt1() {
    static volatile int lastpid = 0;
    uint16_t counter = 0;
    while (true) {
        push_interrupt_disable();
        volatile int test = 5;
        if (counter == 0) {
            kprintf(KP_INFO, "hi from kernel thread(PID %d) stack: 0x%p\n", sched::mytask()->pid, &test);
        }
        pop_interrupt_disable();
        counter++;
        if (counter > 2048) {
            counter = 0;
        }
        int mypid = sched::mytask()->pid;
        lastpid = mypid;
        while (mypid == lastpid) {
            asm volatile("hlt");
        }
        //sched::yield();
    }
}

void arch::startup::kthread0() {
    void *elfptr = nullptr;

    if (fs::vfs::fptr("/usr/lib/modules/module.o", &elfptr)) {
        kprintf(KP_INFO, "archinit: loading kernel module\n");
        elf::load_module(elfptr);
    }

    std::vector<std::string> args;
    if (fs::vfs::fptr("/bin/sh", &elfptr)) {
        args.push_back("/bin/sh");
        elf::load_program(elfptr, &args);
    }

    sched::start_thread(kt1);
    sched::start_thread(kt1);

    // a bit of a hack.. we have to call the vector destructor before killing this process
    // args.~vector();

    multitasking::initMultitasking(); // this will kill this process
}
