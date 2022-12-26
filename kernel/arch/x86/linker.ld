ENTRY(_kentry)

PAGE = 0x1000;
HIGHER_HALF = 0xC0000000;

MEMORY
{
	ram (rwx) : org = 0xC0000000, l = 0xF0000
}

SECTIONS {
	. = HIGHER_HALF;

	.entry ALIGN(PAGE) : AT(ADDR(.entry) - HIGHER_HALF) {
		KEEP(*(.entry))
	}>ram

	.text ALIGN(PAGE) : AT(ADDR(.text) - HIGHER_HALF) {
		KEEP(*(.text))
	}>ram

	.rodata ALIGN(PAGE) : AT(ADDR(.rodata) - HIGHER_HALF) {
		KEEP(*(.rodata))
	}>ram

	.data ALIGN(PAGE) : AT(ADDR(.data) - HIGHER_HALF) {
		START_CONSTRUCTORS = .;
    	KEEP(*( .init_array ));
    	KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    	END_CONSTRUCTORS = .;
		KEEP(*(.data))
		KEEP(*(.sym_table))
		PROVIDE(symtab_start = .);
		PROVIDE(symtab_end = .);
	}>ram

	.bss ALIGN(PAGE) : AT(ADDR(.bss) - HIGHER_HALF) {
		KEEP(*(.bss))
	}>ram

	KERNEL_SIZE = . - HIGHER_HALF;
}
