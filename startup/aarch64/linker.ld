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
        *(.bss)
    }   
}
