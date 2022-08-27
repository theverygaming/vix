all: img

img:
	@$(MAKE) --no-print-directory -C startup
	@$(MAKE) --no-print-directory -C kernel
	@$(MAKE) --no-print-directory -C shitshell
	@cat startup/startup kernel/kernel /dev/zero | dd status=none iflag=fullblock of=kernel.bin bs=65536 count=246 # make the shitshell executable land right at KERNEL_FREE_AREA_BEGIN_OFFSET
	@cat kernel.bin shitshell/shitshell /dev/zero | dd status=none iflag=fullblock of=kernel_shitshell.bin bs=65536 count=290
	@grub/createimg.sh

clean:
	@rm -f shitOS.img shitOS_app.img *.o
	@$(MAKE) --no-print-directory -C startup clean
	@$(MAKE) --no-print-directory -C kernel clean
	@$(MAKE) --no-print-directory -C shitshell clean