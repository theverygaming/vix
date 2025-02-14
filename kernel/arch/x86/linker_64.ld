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
    ram (rwx) : org = 0xffffffff80000000, l = 0xF0000
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
    }>ram :data

    .bss ALIGN(PAGE) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }>ram : data

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }   
}
