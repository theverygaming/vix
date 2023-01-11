ENTRY(_start)

PAGE = 0x1000;

MEMORY
{
    ram (rwx) : org = 0x80000, l = 0xF0000
}

SECTIONS
{
    . = 0x80000;

    .text ALIGN(PAGE) : {
        KEEP(*(.text.boot))
		*(.text)
	}>ram

    .rodata ALIGN(PAGE) : {
        *(.rodata)
    }>ram

    .data ALIGN(PAGE) : {
        START_CONSTRUCTORS = .;
    	KEEP(*( .init_array ));
    	KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    	END_CONSTRUCTORS = .;
		KEEP(*(.data))
    }>ram

    .bss ALIGN(PAGE) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }>ram
    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }   
}

__bss_size = (__bss_end - __bss_start)>>3;
