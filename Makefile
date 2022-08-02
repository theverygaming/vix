all: img

img:
	$(MAKE) -C boot
	$(MAKE) -C kernel
	$(MAKE) -C shitshell
	cat boot/boot kernel/kernel /dev/zero | dd iflag=fullblock of=shitOS.img bs=512 count=2048
	cat shitOS.img shitshell/shitshell /dev/zero | dd iflag=fullblock of=shitOS_app.img bs=512 count=7738

clean:
	rm -f shitOS.img shitOS_app.img *.o
	$(MAKE) -C boot clean
	$(MAKE) -C kernel clean
	$(MAKE) -C shitshell clean