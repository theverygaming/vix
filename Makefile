all: img

img: bootsect kernelbruh
	cat boot/boot kernel/kernel /dev/zero | dd of=shitOS.img bs=512 count=2048

bootsect:
	$(MAKE) -C boot

kernelbruh:
	$(MAKE) -C kernel

clean:
	rm -f shitOS.img *.o
	$(MAKE) -C boot clean
	$(MAKE) -C kernel clean