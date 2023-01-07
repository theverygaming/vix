#!/bin/bash

set -e

${LD} ${LDFLAGS} -T arch/x86/linker.ld kernel_partial.o -o kernel.o
${NM} --format=bsd -n kernel.o | python3 scripts/gensyms.py > symtab.s
${CXX} ${CXXFLAGS} -c symtab.s -o symtab.os
${LD} ${LDFLAGS} -T arch/x86/linker.ld kernel_partial.o symtab.os -o kernel.o
${OBJCOPY} -O binary kernel.o kernel.bin
