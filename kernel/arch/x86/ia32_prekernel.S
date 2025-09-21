#include <vix/config.h>

.section .entry
.align 8
header_start:
    .long 0xe85250d6                // magic number
    .long 0                         // architecture -> 0 = 32-bit protected mode i386
    .long header_end - header_start // header length

    // checksum
    .long 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    .align 8

    .word 5 // type = 5(framebuffer tag)
    .word 0 // flags
    .long 20 // size
    .long 800 // width
    .long 600 // height
    .long 24 // depth

    .align 8

    .word 6 // type = 6(Module alignment tag) -> force modules to be page aligned
    .word 0 // flags
    .long 8 // size = 8

    .align 8

    // required end tag
    .word 0    // type
    .word 0    // flags
    .long 8    // size
header_end:

.global _prekernel_entry
_prekernel_entry:
_lowmem_entry:
    // disable interrupts, we definitely can't handle them now
    cli

    // check multiboot2 magic
    cmpl $0x36D76289, %eax
    jne .die

    // load page directory
    movl $(kinit_page_directory - CONFIG_KERNEL_HIGHER_HALF), %eax
    movl %eax, %cr3

    // enable 4MiB pages
    movl %cr4, %eax
    or $0x00000010, %eax
    movl %eax, %cr4

    // enable paging
    movl %cr0, %eax
    or $0x80000000, %eax
    movl %eax, %cr0

    // jump to higher half
    // (we don't jump directly because that would generate a relative jump.. Which wouldn't get us to higher half)
    leal _highmem_entry, %eax
	jmp *%eax

.extern _kentry
_highmem_entry:
    // initialize stack pointer and ebp
    movl $kinit_stack_top, %esp
    movl $0, %ebp
    pushl %ebx // multiboot2 info structure pointer

    call _kentry
.die:
    cli
    hlt
    jmp .die

.section .data
.align 4096
kinit_page_directory:
    // identity map first 8MiB
    .long 0b10000011 // present, RW, 4MiB page, physical address 0
    .long 0b10000011 | (1 << 22) // present, RW, 4MiB page, physical address 4MiB
    // nothing mapped until start of higher half
    .rept ((CONFIG_KERNEL_HIGHER_HALF >> 22) - 2)
        .long 0
    .endr
    // HHDM
    .set HHDM_PAGES, (CONFIG_HHDM_SIZE >> 22)
    .set HHDM_PAGE_COUNTER, 0
    .rept HHDM_PAGES
        .long 0b10000011 | (HHDM_PAGE_COUNTER << 22) // present, RW, 4MiB page, physical address -> loop counter * 4MiB
        .set HHDM_PAGE_COUNTER, HHDM_PAGE_COUNTER + 1
    .endr
    // fill rest of table
    .rept ((1024 - (CONFIG_KERNEL_HIGHER_HALF >> 22)) - HHDM_PAGES)
        .long 0
    .endr

.section .bss
.align 4
kinit_stack:
    .skip 8192
    kinit_stack_top:
