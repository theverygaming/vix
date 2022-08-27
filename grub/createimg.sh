#!/bin/bash
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd "$parent_path"


cp ../kernel.bin rootfs/boot/kernel.bin
grub-mkrescue -o ../shitOS.iso rootfs/
#dd if=/dev/zero of=image.img bs=10MB count=1 # create empty 10MB file
#mkfs.ext2 image.img # create ext2 filesystem(paritionless)
