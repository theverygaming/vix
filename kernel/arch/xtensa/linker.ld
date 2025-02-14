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
    dram ( RW )       : ORIGIN = 0x3FFF0000, len = 0x70001
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
        START_CONSTRUCTORS_INITARR = .;
        KEEP(*(.init_array));
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)));
        END_CONSTRUCTORS_INITARR = .;
        START_CONSTRUCTORS_CTORS = .;
        KEEP(*(.ctors));
        KEEP(*(SORT_BY_INIT_PRIORITY(.ctors.*)));
        END_CONSTRUCTORS_CTORS = .;
        START_INITCALL_LEVEL_0 = .;
        KEEP(*(.initcall0));
        END_INITCALL_LEVEL_0 = .;
        START_INITCALL_LEVEL_1 = .;
        KEEP(*(.initcall1));
        END_INITCALL_LEVEL_1 = .;
        KEEP(*(.data))
        KEEP(*(.sym_table))
        PROVIDE(symtab_start = .);
        PROVIDE(symtab_end = .);
    }>dram :data

    .bss ALIGN(16) : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }>dram : data

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}

__bss_size = __bss_end - __bss_start;
