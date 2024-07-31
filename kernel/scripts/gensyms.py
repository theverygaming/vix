#!/usr/bin/python3
import sys

print("#include <vix/config.h>\n.section .sym_table\n.global symtab_start\nsymtab_start:\n#ifdef CONFIG_ENABLE_SYMBOL_TABLE")

for line in sys.stdin:
    line = line.rstrip()
    split = line.split()
    symname = split[2]
    symadr = split[0]
    print(f"{sys.argv[1]} 0x{symadr}\n.ascii \"{symname}\\0\"")

print(f"{sys.argv[1]} 0x0\n.ascii \"\\0\"")
print("#endif // CONFIG_ENABLE_SYMBOL_TABLE\n.global symtab_end\nsymtab_end:")
