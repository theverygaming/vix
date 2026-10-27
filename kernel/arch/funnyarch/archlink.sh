#!/usr/bin/env bash
set -eu

${INT_LD} ${INT_LDFLAGS} -T arch/funnyarch/linker.json kernel_partial.o -o kernel.o
cp kernel.o kernel.bin
