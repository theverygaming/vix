#!/usr/bin/env bash
set -eu

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd "$parent_path"

LIMINE_PATH=$(limine --print-datadir)

rm -f image.img
rm -f image_extracted.img

dd if=/dev/zero of=image.img bs=1M count=10 # create empty 10MB disk image

# https://superuser.com/a/984637
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk image.img
  o    # create DOS partition table
  n    # new partition
  p    # primary partition
  1    # partition number 1
  2048 # start partition at 2048
       # default - end of disk
  a    # set bootable flag on created partition
  p    # print partition table
  w    # write the partition table
  q    # quit
EOF

dd if=image.img of=image_extracted.img bs=512 skip=2048 # extract partition
mkfs.fat image_extracted.img

mmd -i image_extracted.img ::boot

mcopy -i image_extracted.img rootfs-aarch64/boot/limine.conf ::boot/
mcopy -i image_extracted.img ../kernel/kernel.o ::boot/kernel.o
mcopy -i image_extracted.img ../roramfs.fs ::boot/initramfs.bin

mmd -i image_extracted.img ::EFI
mmd -i image_extracted.img ::EFI/BOOT
mcopy -i image_extracted.img ${LIMINE_PATH}/BOOTAA64.EFI ::EFI/BOOT/BOOTAA64.EFI

dd if=image_extracted.img of=image.img bs=512 seek=2048 # write partition back to image
cp image.img ../vix_uefi.img
rm -f image.img
rm -f image_extracted.img
