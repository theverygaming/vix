all: img

img: bootsect kernelbruh shitshellbruh
	cat boot/boot kernel/kernel /dev/zero | dd of=shitOS.img bs=512 count=2048
	cat shitOS.img shitshell/shitshell /dev/zero | dd of=shitOS_app.img bs=512 count=4002

bootsect:
	$(MAKE) -C boot

kernelbruh:
	$(MAKE) -C kernel

shitshellbruh:
	$(MAKE) -C shitshell

clean:
	rm -f shitOS.img shitOS_app.img *.o
	$(MAKE) -C boot clean
	$(MAKE) -C kernel clean
	$(MAKE) -C shitshell clean