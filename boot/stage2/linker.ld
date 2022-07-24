ENTRY(start)
OUTPUT_FORMAT("binary")
phys = 0x1000;

MEMORY
  {
    ram (rwx) : org = 0x1000, l = 2M
  }

SECTIONS
{
    . = phys;

    .entry              : { __entry_start = .;      *(.entry)   }
    .text               : { __text_start = .;       *(.text)    }>ram
    .data               : { __data_start = .;       *(.data)    }>ram
    .rodata             : { __rodata_start = .;     *(.rodata)  }>ram
    .bss                : { __bss_start = .;        *(.bss)     }>ram
    
    __end = .;
}