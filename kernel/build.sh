#!/bin/bash

set -e # return if command failed

scripts/shbuild shbuild
if [ "${MAKE_ARCH}" == "x86" ]; then # build symbol table and link it
    nm --format=bsd -n kernel.o | python3 scripts/gensyms.py > symtab.s
    g++ -O3 -Iinclude -nostdlib -m32 -march=i386 -fno-pie -mno-red-zone -fno-stack-protector -ffreestanding -fno-exceptions -Wall -Wextra -Wno-unused-parameter -c symtab.s -o symtab.os
    ld -nostdlib -m elf_i386 -z noexecstack -T arch/x86/linker.ld kernel_partital.o symtab.os -o kernel.o
    objcopy -O binary kernel.o kernel.bin
fi
