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

    .text ALIGN(2) : {
        *(.entry)
		*(.text)
        KEEP(*(.tests))
	}>ram :text

    .rodata ALIGN(2) : {
        *(.rodata)
    }>ram :rodata

    .data ALIGN(2) : {
        START_CONSTRUCTORS = .;
    	KEEP(*( .init_array ));
    	KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    	END_CONSTRUCTORS = .;
		KEEP(*(.data))
        KEEP(*(.sym_table))
        PROVIDE(symtab_start = .);
        PROVIDE(symtab_end = .);
    }>ram :data

    .bss ALIGN(2) : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }>ram : data

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
    __kernel_end = .;
}

__bss_size = __bss_end - __bss_start;
__kernel_size = __kernel_end - PHYS_START;
