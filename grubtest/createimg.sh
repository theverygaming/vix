#!/bin/bash
nasm -f bin helloworld.asm -o hello.o

dd if=/dev/zero of=image.img bs=10MB count=1 # create empty 10MB file
mkfs.ext2 image.img # create ext2 filesystem(paritionless)
