#!/bin/bash
nasm -f bin helloworld.asm -o hello.o

# create FS
#mkdir rootfs
#mkdir rootfs/boot
#mkdir rootfs/boot/grub
cp hello.o rootfs/boot/kernel
grub-mkrescue -o boot.iso rootfs/
qemu-system-x86_64 boot.iso -debugcon stdio
#dd if=/dev/zero of=image.img bs=10MB count=1 # create empty 10MB file
#mkfs.ext2 image.img # create ext2 filesystem(paritionless)
