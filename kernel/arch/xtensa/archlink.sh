#!/usr/bin/env bash
set -e

${INT_LD} ${INT_LDFLAGS} -T arch/xtensa/linker.ld kernel_partial.o -o kernel.o
${INT_NM} --format=bsd -n kernel.o | python3 scripts/gensyms.py .long > symtab.S
${INT_CXX} ${INT_CXXFLAGS} -c symtab.S -o symtab.oS
${INT_LD} ${INT_LDFLAGS} -T arch/xtensa/linker.ld kernel_partial.o symtab.oS -o kernel.o

${INT_NM} --format=bsd -n kernel.o | python3 scripts/gensyms.py .long > symtab.S
${INT_CXX} ${INT_CXXFLAGS} -c symtab.S -o symtab.oS
${INT_LD} ${INT_LDFLAGS} -T arch/xtensa/linker.ld kernel_partial.o symtab.oS -o kernel.o
