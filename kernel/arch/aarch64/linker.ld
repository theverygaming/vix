ENTRY(_start)

MEMORY
{
    ram (rwx) : org = 0x80000, l = 0xF0000
}

SECTIONS
{
    . = 0x80000;

    .text : {
        KEEP(*(.text.boot))
		*(.text)
	}>ram

    .rodata : {
        *(.rodata)
    }>ram

    .data : {
        START_CONSTRUCTORS = .;
    	KEEP(*( .init_array ));
    	KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    	END_CONSTRUCTORS = .;
		KEEP(*(.data))
    }>ram

    .bss : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }>ram
    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }   
}

__bss_size = (__bss_end - __bss_start)>>3;
