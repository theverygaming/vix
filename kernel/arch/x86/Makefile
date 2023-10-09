obj-d += syscalls

obj-$(CONFIG_ENABLE_NETWORKING) += drivers/net/rtl8139.ocpp
obj-$(CONFIG_ENABLE_FTRACE) += trace.ocpp
obj-$(CONFIG_ENABLE_STACKPROTECTOR) += stackprotector.ocpp

obj-y += drivers/ps2.ocpp drivers/pci.ocpp drivers/pic_8259.ocpp drivers/rtc.ocpp drivers/serial.ocpp
obj-y += generic/cpu.ocpp generic/devices.ocpp generic/textoutput.ocpp
obj-y += cpubasics.ocpp cpuid.ocpp elf.ocpp gdt.ocpp modelf.ocpp multitasking.ocpp sched.ocpp multitasking.oS simd.ocpp tss.ocpp

obj-$(CONFIG_ENABLE_KERNEL_32) += drivers/text80x25.ocpp
obj-$(CONFIG_ENABLE_KERNEL_32) += paging.onasm isr.onasm simd.onasm gdt.onasm idt.onasm
obj-$(CONFIG_ENABLE_KERNEL_32) += archinit_32.ocpp multiboot2.ocpp memorymap.ocpp isr.ocpp isrs.ocpp ldt.ocpp idt.ocpp paging_32.ocpp syscall_32.oS syscall_32.ocpp syscalls_32.ocpp except_32.oS except_32.ocpp

obj-$(CONFIG_ENABLE_KERNEL_64) += archinit_64.ocpp paging_64.ocpp

extratargets-$(CONFIG_ENABLE_KERNEL_64) += include/arch/limine.h

$(objtree)/include/arch/limine.h:
	@curl https://raw.githubusercontent.com/limine-bootloader/limine/trunk/limine.h -o $@
	@sed -i 's/#include <stdint.h>/#include <types.h>\n#define NULL (void *)0/g' $@
