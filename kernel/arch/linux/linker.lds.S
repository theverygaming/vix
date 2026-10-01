#include <vix/config.h>
ENTRY(_kentry)

PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata  PT_LOAD    FLAGS((1 << 2)) ;            /* Read only */
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
}

SECTIONS
{
    . = CONFIG_KERNEL_HIGHER_HALF;

    .text ALIGN(CONFIG_ARCH_PAGE_SIZE) : {
        __text_start = .;
		*(.text*)
        KEEP(*(.tests))

        . = ALIGN(CONFIG_ARCH_PAGE_SIZE);
        __monitor_utils_start = .;
        KEEP(*(.monitor_utils))
        . = ALIGN(CONFIG_ARCH_PAGE_SIZE);
        __monitor_utils_end = .;
        __text_end = .;
	} :text

    .rodata ALIGN(CONFIG_ARCH_PAGE_SIZE) : {
        __rodata_start = .;
        *(.rodata*)
        __rodata_end = .;
    } :rodata

    .data ALIGN(CONFIG_ARCH_PAGE_SIZE) : {
        __data_start = .;
        KEEP(*(.data))
        INCLUDE arch/common_data.ld
        __data_end = .;
    } :data

    .bss ALIGN(CONFIG_ARCH_PAGE_SIZE) : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    } :data

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }   
}
