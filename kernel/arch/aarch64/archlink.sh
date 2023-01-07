#!/bin/bash
set -e

${LD} ${LDFLAGS} -T arch/aarch64/linker.ld kernel_partial.o -o kernel.o
