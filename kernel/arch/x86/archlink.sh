#!/bin/bash

set -e

source .config

ldscript="arch/x86/linker_32.ld"

if [ "$CONFIG_ENABLE_KERNEL_64" == "y" ]; then
    ldscript="arch/x86/linker_64.ld"
fi

${LD} ${LDFLAGS} -T ${ldscript} kernel_partial.o -o kernel.o
${NM} --format=bsd -n kernel.o | python3 scripts/gensyms.py > symtab.s
${CXX} ${CXXFLAGS} -c symtab.s -o symtab.os
${LD} ${LDFLAGS} -T ${ldscript} kernel_partial.o symtab.os -o kernel.o
${OBJCOPY} -O binary kernel.o kernel.bin
