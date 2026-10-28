#!/usr/bin/env bash
set -eu

# NOTE: usually we'd use the linker and use PROVIDE but the funnyarch linker doesn't support that :c
echo -e ".section .sym_table\n.global symtab_start\nsymtab_start:\n.global symtab_end\nsymtab_end:" > symtab.S
${INT_CXX} ${INT_CXXFLAGS} -c symtab.S -o symtab.o

${INT_LD} ${INT_LDFLAGS} -T arch/funnyarch/linker.json kernel_partial.o symtab.o -o kernel.o

${INT_NM} --format=bsd -n kernel.o | grep -v ".L" | python3 scripts/gensyms.py .word > symtab.S
${INT_CXX} ${INT_CXXFLAGS} -c symtab.S -o symtab.o
${INT_LD} ${INT_LDFLAGS} -T arch/funnyarch/linker.json kernel_partial.o symtab.o -o kernel.o

${INT_NM} --format=bsd -n kernel.o | grep -v ".L" | python3 scripts/gensyms.py .word > symtab.S
${INT_CXX} ${INT_CXXFLAGS} -c symtab.S -o symtab.o
${INT_LD} ${INT_LDFLAGS} -T arch/funnyarch/linker.json kernel_partial.o symtab.o -o kernel.o

${INT_OBJCOPY} -O binary kernel.o kernel.bin
