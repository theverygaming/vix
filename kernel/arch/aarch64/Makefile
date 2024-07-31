obj-$(CONFIG_ENABLE_STACKPROTECTOR) += stackprotector.ocpp

obj-d += drivers

obj-y += generic/cpu.ocpp generic/devices.ocpp generic/memory.ocpp
obj-y += archinit.ocpp memmap.ocpp exceptions.ocpp sched.ocpp sched.oS


extratargets-y += include/vix/arch/limine.h

$(objtree)/include/vix/arch/limine.h:
	@curl https://raw.githubusercontent.com/limine-bootloader/limine/trunk/limine.h -o $@
	@sed -i 's/#include <stdint.h>/#include <vix/types.h>\n#define NULL (void *)0/g' $@
