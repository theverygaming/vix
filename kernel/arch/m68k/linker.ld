ENTRY(_kentry)

PAGE = 0x1000;

PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata  PT_LOAD    FLAGS((1 << 2)) ;            /* Read only */
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
}

KERNELMEMORY = 90000;

MEMORY
{
    ram (rwx) : org = 0x11000, l = KERNELMEMORY
}

SECTIONS
{
    . = 0x11000;

    .text ALIGN(2) : {
        *(.entry)
		*(.text)
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
    }>ram :data

    .bss ALIGN(2) : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }>ram : data

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }   
}

__bss_size = (__bss_end - __bss_start);
