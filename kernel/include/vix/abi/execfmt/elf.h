#pragma once
#include <vix/arch/common/paging.h>
#include <vix/fs/vfs.h>
#include <memory>
#include <utility>
#include <vix/status.h>
#include <vix/types.h>
#include <vix/config.h>

namespace execfmt::elf {
    /*
     * Returns entry point on success.
     * Assumes the pt is already loaded.
     */ 
    status::StatusOr<void *> load_elf32(arch::vmm::pt_t pt, std::shared_ptr<struct vfs::vnode> elf_file);

    /*
     * Returns entry point on success.
     * Assumes the pt is already loaded.
     */ 
    status::StatusOr<void *> load_elf64(arch::vmm::pt_t pt, std::shared_ptr<struct vfs::vnode> elf_file);

    inline const unsigned int PT_LOAD = 1;
#ifdef CONFIG_ENABLE_EXECFMT_ELF32
    inline const unsigned int ELFCLASS32 = 1;

    struct elf32_header {
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

    struct elf32_program_header {
        uint32_t p_type;
        uint32_t p_offset; // offset of data in elf image
        uint32_t p_vaddr;  // virtual load address
        uint32_t p_paddr;  // physical load address, not used / undefined
        uint32_t p_filesz; // size of data in elf image
        uint32_t p_memsz;  // size of data in memory; any excess over disk size is zero'd
        uint32_t p_flags;
        uint32_t p_align; // alignment
    };
#endif
#ifdef CONFIG_ENABLE_EXECFMT_ELF64
    inline const unsigned int ELFCLASS64 = 2;

    struct elf64_header {
        unsigned char e_ident[16]; // should start with [0x7f 'E' 'L' 'F']
        uint16_t e_type;
        uint16_t e_machine;
        uint32_t e_version;
        uint64_t e_entry;
        uint64_t e_phoff; // start of program headers in file
        uint64_t e_shoff;
        uint32_t e_flags;
        uint16_t e_ehsize;
        uint16_t e_phentsize; // size of each program header
        uint16_t e_phnum; // number of program headers
        uint16_t e_shentsize;
        uint16_t e_shnum;
        uint16_t e_shstrndx;
    };

    struct elf64_program_header {
        uint32_t p_type;
        uint32_t p_flags;
        uint64_t p_offset; // offset of data in elf image
        uint64_t p_vaddr; // virtual load address
        uint64_t p_paddr; // reserved
        uint64_t p_filesz; // size of data in elf image
        uint64_t p_memsz; // size of data in memory; any excess over disk size is zero'd
        uint64_t p_align; // alignment
    };
#endif
}
