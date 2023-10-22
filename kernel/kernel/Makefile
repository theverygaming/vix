obj-y += debug.ocpp framebuffer.ocpp kernel.ocpp psf.ocpp sched.ocpp stdio.ocpp time.ocpp kprintf.ocpp symbols.ocpp interrupts.ocpp

obj-$(CONFIG_ENABLE_BUILTIN_FONT) += unifont.bin.ocpp

$(objtree)/%.bin.cpp: ../fonts/Unifont-APL8x16-15.0.01.psf
	@echo "  BIN2C $<"
	@xxd -i $< $@
