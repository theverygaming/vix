#!/usr/bin/env bash
set -eu

source .config

if [ "${CONFIG_XTENSA_PLATFORM_ESP32:-}" == "y" ]; then
    ldscript="arch/xtensa/linker_esp32.ld"
fi
if [ "${CONFIG_XTENSA_PLATFORM_ESP8266:-}" == "y" ]; then
    ldscript="arch/xtensa/linker_esp8266.ld"
fi

${INT_LD} ${INT_LDFLAGS} -T "${ldscript}" kernel_partial.o -o kernel.o
${INT_NM} --format=bsd -n kernel.o | python3 scripts/gensyms.py .long > symtab.S
${INT_CXX} ${INT_CXXFLAGS} -c symtab.S -o symtab.o
${INT_LD} ${INT_LDFLAGS} -T "${ldscript}" kernel_partial.o symtab.o -o kernel.o

${INT_NM} --format=bsd -n kernel.o | python3 scripts/gensyms.py .long > symtab.S
${INT_CXX} ${INT_CXXFLAGS} -c symtab.S -o symtab.o
${INT_LD} ${INT_LDFLAGS} -T "${ldscript}" kernel_partial.o symtab.o -o kernel.o
