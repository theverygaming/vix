all: img

img:
	@$(MAKE) --no-print-directory -C boot
	@$(MAKE) --no-print-directory -C kernel
	@$(MAKE) --no-print-directory -C shitshell
	@cat boot/boot kernel/kernel /dev/zero | dd status=none iflag=fullblock of=shitOS.img bs=512 count=2048
	@cat shitOS.img shitshell/shitshell /dev/zero | dd status=none iflag=fullblock of=shitOS_app.img bs=512 count=7738

clean:
	@rm -f shitOS.img shitOS_app.img *.o
	@$(MAKE) --no-print-directory -C boot clean
	@$(MAKE) --no-print-directory -C kernel clean
	@$(MAKE) --no-print-directory -C shitshell clean