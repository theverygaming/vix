#!/usr/bin/env bash
set -eu

source .config

ldscript="arch/x86/linker_32.lds"
ptrsize=".long"

if [ "${CONFIG_ENABLE_KERNEL_64:-}" == "y" ]; then
    ldscript="arch/x86/linker_64.ld"
    ptrsize=".quad"
fi

${INT_LD} ${INT_LDFLAGS} -T ${ldscript} kernel_partial.o -o kernel.o
${INT_NM} --format=bsd -n kernel.o | python3 scripts/gensyms.py ${ptrsize} > symtab.S
${INT_CXX} ${INT_CXXFLAGS} -c symtab.S -o symtab.o
${INT_LD} ${INT_LDFLAGS} -T ${ldscript} kernel_partial.o symtab.o -o kernel.o

${INT_NM} --format=bsd -n kernel.o | python3 scripts/gensyms.py ${ptrsize} > symtab.S
${INT_CXX} ${INT_CXXFLAGS} -c symtab.S -o symtab.o
${INT_LD} ${INT_LDFLAGS} -T ${ldscript} kernel_partial.o symtab.o -o kernel.o
