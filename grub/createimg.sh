#!/bin/bash
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd "$parent_path"


cp ../kernel_shitshell.bin rootfs/boot/kernel.bin
cp ../roramfs.fs rootfs/boot/initramfs.bin
grub-mkrescue -o ../shitOS.iso rootfs/
exit
limine_path=../../limine/bin # relative from the location of this script

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
mkfs.ext2 image_extracted.img

if [ -z ${limine_path+x} ]; then
    e2cp /usr/share/limine/limine.sys -G 0 -O 0 image_extracted.img:/boot/
else
    e2cp ${limine_path}/limine.sys -G 0 -O 0 image_extracted.img:/boot/
fi

e2cp rootfs/boot/limine.cfg -G 0 -O 0 image_extracted.img:/boot/
e2cp ../kernel_shitshell.bin -G 0 -O 0 image_extracted.img:/boot/kernel.bin
e2cp ../roramfs.fs -G 0 -O 0 image_extracted.img:/boot/initramfs.bin

dd if=image_extracted.img of=image.img bs=512 seek=2048 # write partition back to image
${limine_path}/limine-deploy image.img
cp image.img ../shitOS.img
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

if [ -z ${limine_path+x} ]; then
    mcopy -i image_extracted.img /usr/share/limine/limine.sys ::boot/
else
    mcopy -i image_extracted.img ${limine_path}/limine.sys ::boot/
fi

mcopy -i image_extracted.img rootfs/boot/limine.cfg ::boot/
mcopy -i image_extracted.img ../kernel_shitshell.bin ::boot/kernel.bin
mcopy -i image_extracted.img ../roramfs.fs ::boot/initramfs.bin

mmd -i image_extracted.img ::EFI
mmd -i image_extracted.img ::EFI/BOOT
mcopy -i image_extracted.img ${limine_path}/BOOTX64.EFI ::EFI/BOOT/BOOTX64.EFI

dd if=image_extracted.img of=image.img bs=512 seek=2048 # write partition back to image
${limine_path}/limine-deploy image.img
cp image.img ../shitOS_uefi.img
rm -f image.img
rm -f image_extracted.img
