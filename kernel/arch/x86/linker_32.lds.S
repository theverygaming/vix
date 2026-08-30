#include <vix/config.h>
ENTRY(_prekernel_entry)

PAGE = 0x1000;

/* load the kernel into high memory right at 1MiB */
KERNEL_PHYS_START = CONFIG_PHYS_ADDRESS;
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
        KEEP(*(.data))
        INCLUDE arch/common_data.ld
    }

    .bss ALIGN(PAGE) : AT(ADDR(.bss) - HIGHER_HALF) {
        _bss_start = ABSOLUTE(.);
        KEEP(*(.bss))
        _bss_end = ABSOLUTE(.);
    }

    _kcode_end = ABSOLUTE(.);
}
