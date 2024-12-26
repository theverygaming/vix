obj-d += ksh

obj-y += kernel.ocpp sched.ocpp stdio.ocpp time.ocpp kprintf.ocpp interrupts.ocpp elf.ocpp initcall.ocpp

obj-$(CONFIG_FB_SUPPORT) += framebuffer.ocpp psf.ocpp

obj-$(CONFIG_ENABLE_BUILTIN_FONT) += unifont.bin.ocpp

obj-$(CONFIG_ENABLE_SYMBOL_TABLE) += symbols.ocpp

$(objtree)/%.bin.cpp: ../fonts/Unifont-APL8x16-15.0.01.psf
	@echo "  BIN2C $<"
	@xxd -i $< $@
