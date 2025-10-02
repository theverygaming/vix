extratargets-$(CONFIG_ENABLE_KERNEL_32) += linker_32.lds
obj-$(CONFIG_ENABLE_NETWORKING) += drivers/net/rtl8139.o
obj-$(CONFIG_ENABLE_FTRACE) += trace.o
obj-$(CONFIG_ENABLE_STACKPROTECTOR) += stackprotector.o

obj-y += drivers/ps2.o drivers/pci.o drivers/pic_8259.o drivers/rtc.o drivers/serial.o
obj-y += generic/cpu.o generic/devices.o
obj-y += cpubasics.o cpuid.o elf.o gdt.o modelf.o multitasking.o sched.o multitasking_s.o simd.o tss.o io.o

obj-$(CONFIG_ENABLE_KERNEL_32) += drivers/text80x25.o
obj-$(CONFIG_ENABLE_KERNEL_32) += paging.o isr_s.o simd_s.o gdt_s.o idt_s.o
obj-$(CONFIG_ENABLE_KERNEL_32) += archinit_32.o multiboot2.o memorymap.o isr.o isrs.o ldt.o idt.o paging_32.o syscall_32_s.o syscall_32.o except_32_s.o except_32.o
obj-$(CONFIG_ENABLE_KERNEL_32) += libgcc.o
obj-$(CONFIG_ENABLE_KERNEL_32) += ia32_prekernel.o

obj-$(CONFIG_ENABLE_KERNEL_64) += archinit_64.o paging_64.o isr_64.o

extratargets-$(CONFIG_ENABLE_KERNEL_64) += include/vix/arch/limine.h

$(objtree)/include/vix/arch/limine.h:
	@curl https://raw.githubusercontent.com/limine-bootloader/limine/5ed5674868183107ea4f0a42b34799af6fe8efee/limine.h -o $@
	@sed -i 's/#include <stdint.h>/#include <vix\/types.h>\n#define NULL (void *)0/g' $@
