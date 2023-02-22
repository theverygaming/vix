#!/bin/bash
set -e

${INT_LD} ${INT_LDFLAGS} -T arch/aarch64/linker.ld kernel_partial.o -o kernel.o
