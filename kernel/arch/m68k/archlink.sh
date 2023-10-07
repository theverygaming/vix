#!/usr/bin/env bash
set -e

${INT_LD} ${INT_LDFLAGS} -T arch/m68k/linker.ld kernel_partial.o -o kernel.o
${INT_NM} --format=bsd -n kernel.o | python3 scripts/gensyms.py .long > symtab.s
${INT_CXX} ${INT_CXXFLAGS} -c symtab.s -o symtab.os
${INT_LD} ${INT_LDFLAGS} -T arch/m68k/linker.ld kernel_partial.o symtab.os -o kernel.o

${INT_NM} --format=bsd -n kernel.o | python3 scripts/gensyms.py .long > symtab.s
${INT_CXX} ${INT_CXXFLAGS} -c symtab.s -o symtab.os
${INT_LD} ${INT_LDFLAGS} -T arch/m68k/linker.ld kernel_partial.o symtab.os -o kernel.o

${INT_OBJCOPY} -O binary kernel.o kernel.bin
