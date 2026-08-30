ENTRY(_kentry)

PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata  PT_LOAD    FLAGS((1 << 2)) ;            /* Read only */
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
}

MEMORY
{
    /* TODO: use full memmap */
    iram ( RX )       : ORIGIN = 0x40080400, len = 0xC1FFF
    dram ( RW )       : ORIGIN = 0x3FFF0000, len = 0xFFFF
}

SECTIONS
{
    . = 0x40080400;

    .text ALIGN(16) : {
        *(.entry)
		*(.text)
        KEEP(*(.tests))
	}>iram :text

    . = 0x3FFF0000;

    .rodata ALIGN(16) : {
        *(.rodata)
    }>dram :rodata

    .data ALIGN(16) : {
        KEEP(*(.data))
        INCLUDE arch/common_data.ld
    }>dram :data

    .bss ALIGN(16) : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }>dram : data

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}

__bss_size = __bss_end - __bss_start;
