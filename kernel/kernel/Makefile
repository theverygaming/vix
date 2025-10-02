obj-d += ksh

obj-y += kernel.o sched.o stdio.o time.o kprintf.o interrupts.o elf.o initcall.o irq.o panic.o

obj-$(CONFIG_FB_SUPPORT) += framebuffer.o psf.o

obj-$(CONFIG_ENABLE_BUILTIN_FONT) += unifont.bin.o

obj-$(CONFIG_ENABLE_SYMBOL_TABLE) += symbols.o

.INTERMEDIATE: $(objtree)/unifont.bin.cpp
$(objtree)/unifont.bin.cpp: ../fonts/Unifont-APL8x16-15.0.01.psf
	@echo "  BIN2C $<"
	@xxd -i $< $@
