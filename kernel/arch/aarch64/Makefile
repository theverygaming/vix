obj-$(CONFIG_ENABLE_STACKPROTECTOR) += stackprotector.o

obj-d += drivers

obj-y += generic/cpu.o generic/devices.o
obj-y += archinit.o memmap.o exceptions.o sched.o sched_s.o


extratargets-y += include/vix/arch/limine.h

$(objtree)/include/vix/arch/limine.h:
	@curl https://raw.githubusercontent.com/limine-bootloader/limine/5ed5674868183107ea4f0a42b34799af6fe8efee/limine.h -o $@
	@sed -i 's/#include <stdint.h>/#include <vix\/types.h>\n#define NULL (void *)0/g' $@
