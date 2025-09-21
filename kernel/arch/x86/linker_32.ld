/* TODO: run linker scripts trough C preprocessor - like linux does with .lds.S files */
ENTRY(_prekernel_entry)

PAGE = 0x1000;

/* load the kernel into high memory right at 1MiB */
KERNEL_PHYS_START = 0x100000;
HIGHER_HALF = 0xC0000000;

SECTIONS {
	. = HIGHER_HALF + KERNEL_PHYS_START;

	_kcode_start = ABSOLUTE(.);

	.text ALIGN(PAGE) : AT(ADDR(.text) - HIGHER_HALF) {
		KEEP(*(.entry))
		KEEP(*(.text))
		KEEP(*(.tests))
	}

	.rodata ALIGN(PAGE) : AT(ADDR(.rodata) - HIGHER_HALF) {
		KEEP(*(.rodata))
	}

	.data ALIGN(PAGE) : AT(ADDR(.data) - HIGHER_HALF) {
        START_CONSTRUCTORS_INITARR = .;
        KEEP(*(.init_array));
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)));
        END_CONSTRUCTORS_INITARR = .;
        START_CONSTRUCTORS_CTORS = .;
        KEEP(*(.ctors));
        KEEP(*(SORT_BY_INIT_PRIORITY(.ctors.*)));
        END_CONSTRUCTORS_CTORS = .;
		INCLUDE arch/common_initcall.ld
        KEEP(*(.data))
        KEEP(*(.sym_table))
        PROVIDE(symtab_start = .);
        PROVIDE(symtab_end = .);
	}

	.bss ALIGN(PAGE) : AT(ADDR(.bss) - HIGHER_HALF) {
		_bss_start = ABSOLUTE(.);
		KEEP(*(.bss))
		_bss_end = ABSOLUTE(.);
	}

	_kcode_end = ABSOLUTE(.);
}
