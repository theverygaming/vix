ENTRY(main)

PAGE = 0x1000;
HIGHER_HALF = 0xC0000000;

MEMORY
  {
    ram (rwx) : org = 0xC0000000, l = 1M
  }

SECTIONS {
	. = HIGHER_HALF;

	.text ALIGN(PAGE) : AT(ADDR(.text) - HIGHER_HALF) {
		*(.text)
		*(.eh_frame)
	}>ram

	.rodata ALIGN(PAGE) : AT(ADDR(.rodata) - HIGHER_HALF) {
		*(.rodata)
	}>ram

	.data ALIGN(PAGE) : AT(ADDR(.data) - HIGHER_HALF) {
		*(.data)
	}>ram

	.bss ALIGN(PAGE) : AT(ADDR(.bss) - HIGHER_HALF) {
		*(.bss)
	}>ram

	KERNEL_SIZE = . - HIGHER_HALF;
}

