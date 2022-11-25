ENTRY(_start)

SECTIONS
{
    . = 0x80000;

    .text : {
        KEEP(*(.text.boot))
		*(.text)
	}

    .rodata : {
        *(.rodata)
    }

    # TODO: constructors

    .bss : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }
    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }   
}

__bss_size = (__bss_end - __bss_start)>>3;
