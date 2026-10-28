#!/usr/bin/env bash
set -eu

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd "$parent_path"

rm -f image.img
rm -f fs.img

dd if=/dev/zero of=fs.img bs=1M count=1 # create empty 1MB fs image

mkfs.fat -F12 fs.img

mmd -i fs.img ::boot
mcopy -i fs.img rootfs-funnyarch/boot.cfg ::/
mcopy -i fs.img ../kernel/kernel.bin ::boot/kernel.bin
mcopy -i fs.img ../roramfs.fs ::boot/initfs.bin

cat ${LITTLEBOOT_FUNNYARCH} fs.img > image.img
cp image.img ../vix.img
rm -f image.img
rm -f fs.img
