#!/bin/bash
set -e

${INT_LD} ${INT_LDFLAGS} -T arch/m68k/linker.ld kernel_partial.o -o kernel.o
