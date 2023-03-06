#include <arch/modelf.h>
#include <arch/symbols.h>
#include <cppstd/string.h>
#include <cppstd/vector.h>
#include <debug.h>
#include <mm/kmalloc.h>
#include <stdlib.h>

struct __attribute__((packed)) elf32_header {
    unsigned char e_ident[16]; // should start with [0x7f 'E' 'L' 'F']
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff; // start of program headers
    uint32_t e_shoff; // start of section headers
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize; // size of each program header
    uint16_t e_phnum;     // number of program headers
    uint16_t e_shentsize; // section header size in bytes
    uint16_t e_shnum;     // number of entries in section header table
    uint16_t e_shstrndx;  // section header index for string table (if 0xFFFF this is somewhere else, can also be zero in case there is none)
};

struct __attribute__((packed)) elf32_program_header {
    uint32_t p_type;
    uint32_t p_offset; // offset of data in elf image
    uint32_t p_vaddr;  // virtual load address
    uint32_t p_paddr;  // physical load address, not used / undefined
    uint32_t p_filesz; // size of data in elf image
    uint32_t p_memsz;  // size of data in memory; any excess over disk size is zero'd
    uint32_t p_flags;
    uint32_t p_align; // alignment
};

// https://github.com/torvalds/linux/blob/master/include/uapi/linux/elf.h
#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMIC  6
#define SHT_NOTE     7
#define SHT_NOBITS   8
#define SHT_REL      9
#define SHT_SHLIB    10
#define SHT_DYNSYM   11
#define SHT_NUM      12
#define SHT_LOPROC   0x70000000
#define SHT_HIPROC   0x7fffffff
#define SHT_LOUSER   0x80000000
#define SHT_HIUSER   0xffffffff

#define SHF_WRITE          0x1
#define SHF_ALLOC          0x2
#define SHF_EXECINSTR      0x4
#define SHF_RELA_LIVEPATCH 0x00100000
#define SHF_RO_AFTER_INIT  0x00200000
#define SHF_MASKPROC       0xf0000000

#define SHN_UNDEF     0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC    0xff00
#define SHN_HIPROC    0xff1f
#define SHN_LIVEPATCH 0xff20
#define SHN_ABS       0xfff1
#define SHN_COMMON    0xfff2
#define SHN_HIRESERVE 0xffff

struct __attribute__((packed)) elf32_section_header {
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
};

#define R_386_NONE 0
#define R_386_32   1
#define R_386_PC32 2

#define ELF32_R_SYM(x)  ((x) >> 8)
#define ELF32_R_TYPE(x) ((x)&0xff)

struct __attribute__((packed)) elf32_rel {
    uint32_t r_offset;
    uint32_t r_info;
};

struct __attribute__((packed)) elf32_rela {
    uint32_t r_offset;
    uint32_t r_info;
    int32_t r_addend;
};

struct __attribute__((packed)) elf32_symtab {
    uint32_t st_name;
    uint32_t st_value;
    uint32_t st_size;
    uint8_t st_info;
    uint8_t st_other;
    uint16_t st_shndx;
};

uint32_t elf32_get_symbol_value(void *ELF_baseadr, uint32_t sectionindex, uint32_t symindex, bool *offset, char **name) {
    *offset = false;
    struct elf32_header *header = (struct elf32_header *)ELF_baseadr;

    struct elf32_section_header *shdr = (struct elf32_section_header *)(((uint8_t *)ELF_baseadr) + header->e_shoff + (header->e_shentsize * sectionindex));

    char *stringtable_syms = ((char *)((struct elf32_section_header *)(((uint8_t *)ELF_baseadr) + header->e_shoff + (header->e_shentsize * shdr->sh_link)))->sh_offset) + (uintptr_t)ELF_baseadr;
    char *stringtable_sect = ((char *)((struct elf32_section_header *)(((uint8_t *)ELF_baseadr) + header->e_shoff + (header->e_shentsize * header->e_shstrndx)))->sh_offset) + (uintptr_t)ELF_baseadr;

    struct elf32_symtab *symtab_entry = (struct elf32_symtab *)(((uint8_t *)ELF_baseadr) + shdr->sh_offset + (shdr->sh_entsize * symindex));

    const char *sname = &stringtable_syms[symtab_entry->st_name];

    if (symtab_entry->st_shndx == SHN_UNDEF) {
        DEBUG_PRINTF_INSANE("get_sym -- %s\n", sname);
        return syms::get_sym(sname);
    } else if (symtab_entry->st_shndx == SHN_ABS) {
        kprintf(KP_EMERG, "isr: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
        while (true) {}
        return symtab_entry->st_value;
    } else {
        struct elf32_section_header *shdr2 = (struct elf32_section_header *)(((uint8_t *)ELF_baseadr) + header->e_shoff + (header->e_shentsize * symtab_entry->st_shndx));
        *name = &stringtable_sect[shdr2->sh_name];
        *offset = true;
        return symtab_entry->st_value;
    }
    return 0;
}

struct section_alloc {
    std::string name;
    void *address;
};

uint32_t elf32_find_symbol(const char *name, void *ELF_baseadr, std::vector<section_alloc> *sections) {
    struct elf32_header *header = (struct elf32_header *)ELF_baseadr;
    for (int i = 0; i < header->e_shnum; i++) {
        struct elf32_section_header *shdr = (struct elf32_section_header *)(((uint8_t *)ELF_baseadr) + header->e_shoff + (header->e_shentsize * i));
        if (!(shdr->sh_type == SHT_SYMTAB)) {
            continue;
        }
        char *stringtable_syms = ((char *)((struct elf32_section_header *)(((uint8_t *)ELF_baseadr) + header->e_shoff + (header->e_shentsize * shdr->sh_link)))->sh_offset) + (uintptr_t)ELF_baseadr;
        char *stringtable_sect =
            ((char *)((struct elf32_section_header *)(((uint8_t *)ELF_baseadr) + header->e_shoff + (header->e_shentsize * header->e_shstrndx)))->sh_offset) + (uintptr_t)ELF_baseadr;
        for (size_t j = 0; j < shdr->sh_size / shdr->sh_entsize; j++) {
            struct elf32_symtab *symtab_entry = (struct elf32_symtab *)(((uint8_t *)ELF_baseadr) + shdr->sh_offset + (shdr->sh_entsize * j));
            if (strcmp(name, &stringtable_syms[symtab_entry->st_name]) == 0) {
                // find section for this symbol
                struct elf32_section_header *shdr_target = (struct elf32_section_header *)(((uint8_t *)ELF_baseadr) + header->e_shoff + (header->e_shentsize * symtab_entry->st_shndx));
                for (size_t k = 0; k < sections->size(); k++) {
                    if (strcmp((*sections)[k].name.c_str(), &stringtable_sect[shdr_target->sh_name]) == 0) {
                        return (uintptr_t)(*sections)[k].address + symtab_entry->st_value;
                    }
                }
            }
        }
    }
    return 0;
}

void elf::load_module(void *ELF_baseadr) {
    struct elf32_header header;

    memcpy(&header, ELF_baseadr, sizeof(elf32_header));

    if (header.e_ident[0] != 0x7f || memcmp(&header.e_ident[1], "ELF", 3) != 0) {
        DEBUG_PRINTF("Issue: ELF invalid\n");
        return;
    }

    if (header.e_shstrndx == 0xFFFF || header.e_shstrndx == 0) {
        DEBUG_PRINTF("Issue: ELF incompatible\n");
        return;
    }

    // read the section header table and allocate all sections
    std::vector<section_alloc> sections;
    // TODO: investigate pointer overflow
    const char *stringtable_base = ((const char *)((struct elf32_section_header *)(((uint8_t *)ELF_baseadr) + header.e_shoff + (header.e_shentsize * header.e_shstrndx)))->sh_offset) + (uintptr_t)ELF_baseadr;
    for (int i = 0; i < header.e_shnum; i++) {
        struct elf32_section_header *shdr = (struct elf32_section_header *)(((uint8_t *)ELF_baseadr) + header.e_shoff + (header.e_shentsize * i));
        if (!(shdr->sh_flags & SHF_ALLOC)) {
            continue;
        }
        const char *name = stringtable_base + shdr->sh_name;
        void *adr;
        if (shdr->sh_addralign != 0 && shdr->sh_addralign != 1) {
            adr = mm::kmalloc_aligned(shdr->sh_size, shdr->sh_addralign);
        } else {
            adr = mm::kmalloc(shdr->sh_size);
        }
        sections.push_back({.name = name, .address = adr});

        memset(adr, 0, shdr->sh_size);

        if (shdr->sh_type != SHT_NOBITS) {
            memcpy(adr, ((char *)ELF_baseadr) + shdr->sh_offset, shdr->sh_size);
        }
    }

    for (int i = 0; i < header.e_shnum; i++) {
        struct elf32_section_header *shdr = (struct elf32_section_header *)(((uint8_t *)ELF_baseadr) + header.e_shoff + (header.e_shentsize * i));
        const char *name = stringtable_base + shdr->sh_name;
        if (shdr->sh_type == SHT_REL) {
            size_t count = shdr->sh_size / shdr->sh_entsize;
            for (size_t j = 0; j < count; j++) {
                struct elf32_rel *reloc = (struct elf32_rel *)(((char *)ELF_baseadr) + shdr->sh_offset + (j * shdr->sh_entsize));
                struct elf32_section_header *target_shdr = (struct elf32_section_header *)(((uint8_t *)ELF_baseadr) + header.e_shoff + (header.e_shentsize * shdr->sh_info));

                void *target_address = nullptr;

                for (size_t i = 0; i < sections.size(); i++) {
                    if (strcmp(sections[i].name.c_str(), &stringtable_base[target_shdr->sh_name]) == 0) {
                        target_address = sections[i].address;
                        break;
                    }
                }
                if (target_address == nullptr) {
                    continue; // required to be able to load binaries with debug symbols
                    DEBUG_PRINTF("target_address == nullptr\n");
                    return;
                }

                uintptr_t *rel_sym = (uintptr_t *)(((uint8_t *)target_address) + reloc->r_offset);

                char *symname;
                bool isoffset;
                uintptr_t symval = elf32_get_symbol_value(ELF_baseadr, shdr->sh_link, ELF32_R_SYM(reloc->r_info), &isoffset, &symname);

                if (symval == 0 && !isoffset) { // unable to locate symbol
                    DEBUG_PRINTF("symval == 0 && !isoffset\n");
                    return;
                }

                if (isoffset) {
                    bool set = false;
                    for (size_t i = 0; i < sections.size(); i++) {
                        if (strcmp(sections[i].name.c_str(), symname) == 0) {
                            symval = (uintptr_t)sections[i].address + symval;
                            set = true;
                            break;
                        }
                    }
                    if (!set) {
                        DEBUG_PRINTF("!set\n");
                        return;
                    }
                }

                switch (ELF32_R_TYPE(reloc->r_info)) {
                case R_386_NONE:
                    break;
                case R_386_32:
                    *rel_sym += symval;
                    break;
                case R_386_PC32:
                    *rel_sym += symval - ((uintptr_t)rel_sym);
                    break;
                default:
                    DEBUG_PRINTF("Issue: unsupported relocation type\n");
                    return;
                    break;
                }
            }
        } else if (shdr->sh_type == SHT_RELA) {
            kprintf(KP_INFO, "rela %s\n", name);
            return;
        } else {
            continue;
        }
    }

    char **modauthor = (char **)elf32_find_symbol("__MODULE_AUTHOR", ELF_baseadr, &sections);
    if (modauthor != nullptr) {
        kprintf(KP_INFO, "kmod: module author: %s\n", *modauthor);
    }
    char **moddescription = (char **)elf32_find_symbol("__MODULE_DESCRIPTION", ELF_baseadr, &sections);
    if (moddescription != nullptr) {
        kprintf(KP_INFO, "kmod: module description: %s\n", *moddescription);
    }
    char **modversion = (char **)elf32_find_symbol("__MODULE_VERSION", ELF_baseadr, &sections);
    if (modversion != nullptr) {
        kprintf(KP_INFO, "kmod: module version: %s\n", *modversion);
    }

    int (*modinit)() = (int (*)())elf32_find_symbol("__MODULE_INIT", ELF_baseadr, &sections);
    if (modinit != nullptr) {
        if (modinit() != 0) {
            kprintf(KP_INFO, "kmod: vix module init failed!\n");
        }
    }

    int (*linux_modinit)() = (int (*)())elf32_find_symbol("init_module", ELF_baseadr, &sections);
    if (linux_modinit != nullptr) {
        if (linux_modinit() != 0) {
            kprintf(KP_INFO, "kmod: linux module init failed!\n");
        }
    }

    void (*modexit)() = (void (*)())elf32_find_symbol("__MODULE_EXIT", ELF_baseadr, &sections);
    if (modexit != nullptr) {
        modexit();
    }

    void (*linux_modexit)() = (void (*)())elf32_find_symbol("cleanup_module", ELF_baseadr, &sections);
    if (linux_modexit != nullptr) {
        linux_modexit();
    }
}
