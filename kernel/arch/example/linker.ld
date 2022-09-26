ENTRY(_kentry)

PAGE = 0x1000;
HIGHER_HALF = 0xC0000000;

MEMORY
{
	ram (rwx) : org = 0xC0000000, l = 1500K
}

SECTIONS {
	. = HIGHER_HALF;

	.entry ALIGN(PAGE) : AT(ADDR(.entry) - HIGHER_HALF) {
		*(.entry)
	}>ram

	.text ALIGN(PAGE) : AT(ADDR(.text) - HIGHER_HALF) {
		*(.text)
	}>ram

	.rodata ALIGN(PAGE) : AT(ADDR(.rodata) - HIGHER_HALF) {
		*(.rodata)
	}>ram

	.data ALIGN(PAGE) : AT(ADDR(.data) - HIGHER_HALF) {
		START_CONSTRUCTORS = .;
    	KEEP(*( .init_array ));
    	KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    	END_CONSTRUCTORS = .;
		*(.data)
	}>ram

	.bss ALIGN(PAGE) : AT(ADDR(.bss) - HIGHER_HALF) {
		*(.bss)
	}>ram

	KERNEL_SIZE = . - HIGHER_HALF;
}
