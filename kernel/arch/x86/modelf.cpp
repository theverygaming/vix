#include <arch/x86/modelf.h>
#include <log.h>
#include <stdlib.h>

struct __attribute__((packed)) ElfHeader {
    unsigned char e_ident[16]; // should start with [0x7f 'E' 'L' 'F']
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff; // start of program headers in file
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize; // size of each program header
    uint16_t e_phnum;     // number of program headers
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};

struct __attribute__((packed)) ElfProgramHeader {
    uint32_t p_type;
    uint32_t p_offset; // offset of data in elf image
    uint32_t p_vaddr;  // virtual load address
    uint32_t p_paddr;  // physical load address, not used / undefined
    uint32_t p_filesz; // size of data in elf image
    uint32_t p_memsz;  // size of data in memory; any excess over disk size is zero'd
    uint32_t p_flags;
    uint32_t p_align; // alignment
};

void elf::load_module(void *ELF_baseadr) {
    struct ElfHeader header;

    memcpy(&header, ELF_baseadr, sizeof(ElfHeader));

    if (header.e_ident[0] != 0x7f || memcmp(&header.e_ident[1], "ELF", 3) != 0) {
        LOG_DEBUG("Issue: ELF invalid\n");
    }

    
}
