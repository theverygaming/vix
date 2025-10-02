#!/usr/bin/env bash
set -eu

${INT_LD} ${INT_LDFLAGS} -T arch/xtensa/linker.ld kernel_partial.o -o kernel.o
${INT_NM} --format=bsd -n kernel.o | python3 scripts/gensyms.py .long > symtab.S
${INT_CXX} ${INT_CXXFLAGS} -c symtab.S -o symtab.o
${INT_LD} ${INT_LDFLAGS} -T arch/xtensa/linker.ld kernel_partial.o symtab.o -o kernel.o

${INT_NM} --format=bsd -n kernel.o | python3 scripts/gensyms.py .long > symtab.S
${INT_CXX} ${INT_CXXFLAGS} -c symtab.S -o symtab.o
${INT_LD} ${INT_LDFLAGS} -T arch/xtensa/linker.ld kernel_partial.o symtab.o -o kernel.o
