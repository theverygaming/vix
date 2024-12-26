ENTRY(_kentry)

PAGE = 0x1000;

KERNEL_START = 0x1EA000;
HIGHER_HALF = 0xC0000000;
KERNEL_OFFSET = HIGHER_HALF - KERNEL_START;

MEMORY
{
	ram (rwx) : org = 0xC0000000, l = 0xF0000
}

SECTIONS {
	. = KERNEL_START + KERNEL_OFFSET;

	.entry ALIGN(PAGE) : AT(ADDR(.entry) - KERNEL_OFFSET) {
		KEEP(*(.entry))
	}>ram

	.text ALIGN(PAGE) : AT(ADDR(.text) - KERNEL_OFFSET) {
		KEEP(*(.text))
		KEEP(*(.tests))
	}>ram

	.rodata ALIGN(PAGE) : AT(ADDR(.rodata) - KERNEL_OFFSET) {
		KEEP(*(.rodata))
	}>ram

	.data ALIGN(PAGE) : AT(ADDR(.data) - KERNEL_OFFSET) {
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
        KEEP(*(.data))
        KEEP(*(.sym_table))
        PROVIDE(symtab_start = .);
        PROVIDE(symtab_end = .);
	}>ram

	.bss ALIGN(PAGE) : AT(ADDR(.bss) - KERNEL_OFFSET) {
		_bss_start = ABSOLUTE(.);
		KEEP(*(.bss))
		_bss_end = ABSOLUTE(.);
	}>ram
}
