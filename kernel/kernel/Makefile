obj-y += cpp.ocpp debug.ocpp framebuffer.ocpp kernel.ocpp log.ocpp psf.ocpp scheduler.ocpp stdio.ocpp stdlib.ocpp time.ocpp

obj-$(CONFIG_UBSAN) += ubsan.ocpp

obj-$(CONFIG_ENABLE_BUILTIN_FONT) += unifont.bin.ocpp

$(objtree)/%.bin.cpp: ../fonts/Unifont-APL8x16-15.0.01.psf
	@echo "  BIN2C $<"
	@python3 scripts/bin2c.py file_Unifont_APL8x16_15_0_01_psf < $< > $@
