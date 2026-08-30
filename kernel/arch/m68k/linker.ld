ENTRY(_kentry)

PAGE = 0x1000;

PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata  PT_LOAD    FLAGS((1 << 2)) ;            /* Read only */
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
}

PHYS_START = 0x13000;

MEMORY
{
    ram (rwx) : org = PHYS_START, l = 120000
}

SECTIONS
{
    . = PHYS_START;

    .text ALIGN(4) : {
        *(.entry)
		*(.text)
        KEEP(*(.tests))
	}>ram :text

    .rodata ALIGN(4) : {
        *(.rodata)
    }>ram :rodata

    .data ALIGN(4) : {
        KEEP(*(.data))
        INCLUDE arch/common_data.ld
    }>ram :data

    .bss ALIGN(4) : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }>ram : data

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
    __kernel_end = .;
}

__bss_size = __bss_end - __bss_start;
__kernel_size = __kernel_end - PHYS_START;
