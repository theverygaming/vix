ENTRY(_start)
OUTPUT_FORMAT("binary")
phys = 0x1DA000;

MEMORY
  {
    ram (rwx) : org = phys, l = 0x10000
  }

SECTIONS
{
    . = phys;

    .entry              : { __entry_start = .;      *(.entry)   }>ram
    .text               : { __text_start = .;       *(.text)    }>ram
    .data               : { __data_start = .;       *(.data)    }>ram
    .rodata             : { __rodata_start = .;     *(.rodata)  }>ram
    .bss                : { __bss_start = .;        *(.bss)     }>ram
    
    __end = .;
}