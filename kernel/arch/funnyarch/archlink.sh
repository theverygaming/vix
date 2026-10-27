#!/usr/bin/env bash
set -eu

${INT_LD} ${INT_LDFLAGS} -T arch/funnyarch/linker.json kernel_partial.o -o kernel.o
${INT_OBJCOPY} -O binary kernel.o kernel.bin
