ENTRY(_kentry)

PAGE = 0x1000;

PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata  PT_LOAD    FLAGS((1 << 2)) ;            /* Read only */
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
}

SECTIONS
{
    . = 0xffffffff80000000;

    .text ALIGN(PAGE) : {
		*(.text)
        KEEP(*(.tests))
	} :text

    .rodata ALIGN(PAGE) : {
        *(.rodata)
    } :rodata

    .data ALIGN(PAGE) : {
        KEEP(*(.data))
        INCLUDE arch/common_data.ld
    } :data

    .bss ALIGN(PAGE) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    } : data

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }   
}
