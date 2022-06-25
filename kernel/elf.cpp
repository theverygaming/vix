#include "elf.h"
#include "stdio.h"
#include "stdlib.h"
#include "paging.h"
#include "multitasking.h"

void elf::load_program(void* ELF_baseadr) {
    printf("%c%c%c\n", (char)*((char*)ELF_baseadr + 1), (char)*((char*)ELF_baseadr + 2), (char)*((char*)ELF_baseadr + 3));
    if(*((uint8_t*)ELF_baseadr + 4) == 1) {
        printf("32-bit ELF detected\n");
    }
    if(*((uint8_t*)ELF_baseadr + 5)) {
        printf("litte endian\n");
    }
    if(*((uint8_t*)ELF_baseadr + 16) == 2) {
        printf("executable\n");
    }
    if(*((uint8_t*)ELF_baseadr + 18) == 3) {
        printf("x86\n");
    }

    ElfHeader header;
    ElfProgramHeader pHeader;

    memcpy((char*)&header, (char*)ELF_baseadr, sizeof(ElfHeader));

    printf("e_phoff: 0x%p\n", header.e_phoff);

    // we make a lot of assumptions here
    
    // allocate memory for this one specific binary

    uint32_t max = 0;
    uint32_t min = 0xFFFFFFFF;
    for (int i = 0; i < header.e_phnum; i++) {
        uint32_t addr;
        memcpy((char*)&pHeader, (char*)ELF_baseadr + header.e_phoff + (header.e_phentsize * i), sizeof(pHeader));
        if(pHeader.p_type != 1) {
            continue;
        }
        if(pHeader.p_vaddr + pHeader.p_memsz > max) {
            max = pHeader.p_vaddr + pHeader.p_memsz;
        }
        if(pHeader.p_vaddr < min) {
            min = pHeader.p_vaddr;
        }
    }

    uint32_t initial_physadr = 0x10000000;
    uint32_t virt_base = 134512640;
    uint32_t pagecount = ((max - min) / 4096) + 5;

    for(int i = 0; i < pagecount; i++) {
        paging::map_page((void*)(initial_physadr + (i * 0x1000)), (void*)(virt_base + (i * 0x1000)));
    }
    
    printf("---Program Headers---\n");
    for (int i = 0; i < header.e_phnum; i++) {

        uint32_t addr;
        memcpy((char*)&pHeader, (char*)ELF_baseadr + header.e_phoff + (header.e_phentsize * i), sizeof(pHeader));
        

        if(pHeader.p_type != 1) {
            printf("ignoring section of type: 0x%p\n", pHeader.p_type);
            continue;
        }
        printf("section: align: 0x%p vaddr->0x%p sizef->0x%p sizem->0x%p\n", pHeader.p_align, pHeader.p_vaddr, pHeader.p_filesz, pHeader.p_memsz);
        printf("offset: 0x%p\n", pHeader.p_offset);

        memset((char*)pHeader.p_vaddr, 0, pHeader.p_memsz);
        memcpy((char*)pHeader.p_vaddr, (char*)ELF_baseadr+pHeader.p_offset, pHeader.p_filesz);

        if(pHeader.p_vaddr + pHeader.p_memsz > max) {
            max = pHeader.p_vaddr + pHeader.p_memsz;
        }
        if(pHeader.p_vaddr < min) {
            min = pHeader.p_vaddr;
        }
    }
    printf("max: %u\n", max);
    printf("min: %u\n", min);
    printf("Inital program memory size: %u\n", max - min);

    printf("Entry point: 0x%p\n", header.e_entry);


    multitasking::create_task((void*)(max + (4096 * 4)), (void*)header.e_entry);
}