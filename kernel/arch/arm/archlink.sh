#!/usr/bin/env bash
set -e

${INT_LD} ${INT_LDFLAGS} -T arch/arm/linker.ld kernel_partial.o -o kernel.o
