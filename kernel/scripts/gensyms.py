#!/usr/bin/python3
import sys

print(".section .sym_table\n.global symtab_start\nsymtab_start:")

for line in sys.stdin:
    line = line.rstrip()
    split = line.split()
    symname = split[2]
    symadr = split[0]
    print(f"{sys.argv[1]} 0x{symadr}\n.ascii \"{symname}\\0\"")

print(f"{sys.argv[1]} 0x0\n.ascii \"\\0\"")
print(".global symtab_end\nsymtab_end:")
