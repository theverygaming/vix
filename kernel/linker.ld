ENTRY(main)

PAGE = 0x1000;
HIGHER_HALF = 0xC0000000;

SECTIONS {
	. = HIGHER_HALF;

	.text ALIGN(PAGE) : AT(ADDR(.text) - HIGHER_HALF) {
		*(.text)
		*(.eh_frame)
	}

	.rodata ALIGN(PAGE) : AT(ADDR(.rodata) - HIGHER_HALF) {
		*(.rodata)
	}

	.data ALIGN(PAGE) : AT(ADDR(.data) - HIGHER_HALF) {
		*(.data)
	} 

	.bss ALIGN(PAGE) : AT(ADDR(.bss) - HIGHER_HALF) {
		*(.bss)
	}

	KERNEL_SIZE = . - HIGHER_HALF;
}

