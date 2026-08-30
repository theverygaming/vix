ENTRY(_kentry)

PAGE = 0x1000;

PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata  PT_LOAD    FLAGS((1 << 2)) ;            /* Read only */
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
}

MEMORY
{
    ram (rwx) : org = 0xffffffff80000000, l = 0xFF0000
}

SECTIONS
{
    . = 0xffffffff80000000;

    .text ALIGN(PAGE) : {
		*(.text)
        KEEP(*(.tests))
	}>ram :text

    . += CONSTANT(MAXPAGESIZE);

    .rodata ALIGN(PAGE) : {
        *(.rodata)
    }>ram :rodata

    . += CONSTANT(MAXPAGESIZE);

    .data ALIGN(PAGE) : {
        KEEP(*(.data))
        INCLUDE arch/common_data.ld
    }>ram :data

    .bss ALIGN(PAGE) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }>ram : data

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }   
}

__bss_size = (__bss_end - __bss_start)>>3;
