#!/usr/bin/env bash
set -e

source .config

ldscript="arch/m68k/linker.ld"

if [ "$CONFIG_PLAIN_BINARY" == "y" ]; then
    ldscript="arch/m68k/linker_binary.ld"
fi

${INT_LD} ${INT_LDFLAGS} -T ${ldscript} kernel_partial.o -o kernel.o
${INT_NM} --format=bsd -n kernel.o | python3 scripts/gensyms.py .long > symtab.s
${INT_CXX} ${INT_CXXFLAGS} -c symtab.s -o symtab.os
${INT_LD} ${INT_LDFLAGS} -T ${ldscript} kernel_partial.o symtab.os -o kernel.o

${INT_NM} --format=bsd -n kernel.o | python3 scripts/gensyms.py .long > symtab.s
${INT_CXX} ${INT_CXXFLAGS} -c symtab.s -o symtab.os
${INT_LD} ${INT_LDFLAGS} -T ${ldscript} kernel_partial.o symtab.os -o kernel.o

${INT_OBJCOPY} -O binary kernel.o kernel.bin
