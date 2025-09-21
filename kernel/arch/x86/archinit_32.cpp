#include <vix/mm/mm.h>
#include <string>
#include <vector>
#include <vix/arch/archinit.h>
#include <vix/arch/common/bootup.h>
#include <vix/arch/common/cpu.h>
#include <vix/arch/cpubasics.h>
#include <vix/arch/cpuid.h>
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

static void *mb2_info;
static size_t mb2_info_size;

static void fbputc(char c) {
    fbconsole.fbputc(c);
}

static void *initramfs_start = nullptr;
static size_t initramfs_size = 0;

extern "C" uint8_t _kcode_start;
extern "C" uint8_t _kcode_end;

static void kernelinit(void *multiboot2_info_ptr) {
    drivers::textmode::text80x25::init();
    stdio::set_putc_function(drivers::textmode::text80x25::putc);
    drivers::serial::init();
    stdio::set_putc_function(drivers::serial::putc, true);
    if ((size_t)multiboot2_info_ptr & 7) {
        KERNEL_PANIC("multiboot2 info structure is not aligned, something is wrong here");
    }
    mb2_info = multiboot2_info_ptr;
    mb2_info_size = multiboot2::get_tags_size(mb2_info);
    kprintf(KP_INFO, "archinit: multiboot2 info @ 0x%p size: 0x%p\n", mb2_info, mb2_info_size);
    if (multiboot2::find_initramfs(mb2_info, &initramfs_start, &initramfs_size)) {
        initramfs_size = ALIGN_UP(initramfs_size, CONFIG_ARCH_PAGE_SIZE);
        kprintf(KP_INFO, "archinit: initramfs @ 0x%p size: 0x%p\n", initramfs_start, initramfs_size);
        if (!PTR_IS_ALIGNED(initramfs_start, CONFIG_ARCH_PAGE_SIZE)) {
            initramfs_size = 0;
        }
    }
    int memMap_count;
    void *memMap = multiboot2::findMemMap(mb2_info, &memMap_count);
    memorymap::initMemoryMap(
        memMap,
        memMap_count,
        [](size_t n) -> struct mm::mem_map_entry {
            switch (n) {
                case 0:
                    return {
                        .base = (uintptr_t)&_kcode_start - CONFIG_HHDM_VIRT_BASE,
                        .size = ALIGN_UP((uintptr_t)&_kcode_end - (uintptr_t)&_kcode_start, CONFIG_ARCH_PAGE_SIZE),
                        .type = mm::mem_map_entry::type_t::RESERVED,
                    };
                case 1:
                    return {
                        .base = (uintptr_t)mb2_info,
                        .size = ALIGN_UP(mb2_info_size, CONFIG_ARCH_PAGE_SIZE),
                        .type = mm::mem_map_entry::type_t::RECLAIMABLE,
                    };
                case 2:
                    return {
                        .base = (uintptr_t)initramfs_start,
                        .size = ALIGN_UP(initramfs_size, CONFIG_ARCH_PAGE_SIZE),
                        .type = mm::mem_map_entry::type_t::RECLAIMABLE,
                    };
            }
            KERNEL_PANIC("unreachable");
        },
        initramfs_size > 0 ? 3 : 2
    );
    gdt::init();
    cpubasics::cpuinit_early(); // interrupt handlers are enabled here, before this all exceptions will cause a triplefault
    kernelstart();
}

extern "C" uint8_t _bss_start;
extern "C" uint8_t _bss_end;

extern "C" void __attribute__((section(".entry"))) _kentry(uint32_t multiboot2_info_ptr) {
    size_t sp;
    asm volatile("mov %%esp, %0" : "=a"(sp) :);
    if (sp < KERNEL_VIRT_ADDRESS) {
        return;
    }
    for (uint8_t *addr = &_bss_start; addr < &_bss_end; addr++) {
        *addr = 0;
    }
    kernelinit((void *)multiboot2_info_ptr);
    while (true) {}
}

void arch::startup::stage2_startup() {
    paging::init();
}

void arch::startup::stage3_startup() {
    ASSIGN_OR_PANIC(
        mb2_info,
        mm::map_arbitrary_phys((mm::paddr_t)mb2_info, mb2_info_size)
    );

    framebuffer.init(multiboot2::findFrameBuffer(mb2_info));
    fbconsole.init(&framebuffer);

    if (initramfs_size != 0) {
        ASSIGN_OR_PANIC(
            initramfs_start,
            mm::map_arbitrary_phys((mm::paddr_t)initramfs_start, initramfs_size)
        );
    }
}

void arch::startup::stage4_startup() {
    cpubasics::cpuinit(); // interrupts are enabled here, the timer is also set up here
    cpuid::printFeatures();
    simd::enableSSE();
    if (initramfs_size != 0) {
        if (fs::filesystems::tarfs::init(initramfs_start)) {
            fs::filesystems::tarfs::mountInVFS();
        }
    }
    time::bootupTime = time::getCurrentUnixTime();
    framebuffer.clear();
    fbconsole.init2();
    stdio::set_putc_function(fbputc);
}

static void kt1() {
    uint32_t counter = 0;
    while (true) {
        push_interrupt_disable();
        volatile int test = 5;
        if (counter == 0) {
            kprintf(KP_INFO, "hi from kernel thread(PID %d) stack: 0x%p\n", sched::mytask()->pid, &test);
        }
        pop_interrupt_disable();
        counter++;
        if (counter > 600000) {
            counter = 0;
        }
        sched::yield();
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
