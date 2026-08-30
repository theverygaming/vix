ENTRY(_kentry)

MEMORY
{
    /* TODO: use full memmap */
    iram ( RX )       : ORIGIN = 0x40100000, len = 0x8000
    dram ( RW )       : ORIGIN = 0x3FFE8000, len = 0x14000
}

SECTIONS
{
    . = 0x40100000;

    .text : ALIGN(16) {
        *(.entry)
		*(.text)
        KEEP(*(.tests))
	}>iram

    .rodata : ALIGN(16) {
        *(.rodata)
    }>dram

    .data : ALIGN(16) {
        KEEP(*(.data))
        INCLUDE arch/common_data.ld
    }>dram

    .bss : ALIGN(16) {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }>dram

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}

__bss_size = __bss_end - __bss_start;
