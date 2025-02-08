#include <string.h>
#include <vix/arch/common/paging.h>
#include <vix/arch/generic/memory.h>
#include <vix/arch/paging.h>
#include <vix/config.h>
#include <vix/stdio.h>

uint32_t (*pagetables)[1024] = (uint32_t (*)[1024])(KERNEL_VIRT_ADDRESS + PAGE_TABLES_OFFSET);
uint32_t *page_directory = (uint32_t *)(KERNEL_VIRT_ADDRESS + PAGE_DIRECTORY_OFFSET);

extern "C" void loadPageDirectory(void *address);
extern "C" void enablePaging();

enum page_size { FOUR_KiB, FOUR_MB };
enum page_priv { SUPERVISOR, USER };
enum page_perms { R, RW };

struct directoryEntry {
    void *address;
    enum page_size pagesize;
    bool cache_disabled;
    bool write_through;
    enum page_priv priv;
    enum page_perms perms;
    bool present;
};

struct pagetableEntry {
    void *address;
    bool global;
    bool dirty;
    bool cache_disabled;
    bool write_through;
    enum page_priv priv;
    enum page_perms perms;
    bool present;
};

static uint32_t make_directory_entry(struct directoryEntry de) {
    uint32_t entry = (uint32_t)de.address & 0xFFFFF000;
    entry |= de.pagesize << 7;
    entry |= de.cache_disabled << 4;
    entry |= de.write_through << 3;
    entry |= de.priv << 2;
    entry |= de.perms << 1;
    entry |= de.present;
    return entry;
}

static uint32_t make_table_entry(struct pagetableEntry pe) {
    uint32_t entry = ((uint32_t)pe.address) & 0xFFFFF000;
    entry |= pe.global << 8;
    entry |= pe.dirty << 6;
    entry |= pe.cache_disabled << 4;
    entry |= pe.write_through << 3;
    entry |= pe.priv << 2;
    entry |= pe.perms << 1;
    entry |= pe.present;
    return entry;
}

static inline uint32_t change_table_address(uint32_t entry, void *address) {
    uint32_t ret = ((uint32_t)address) & 0xFFFFF000;
    ret |= entry & 0xFFF;
    return ret;
}

static struct directoryEntry get_directory_entry(uint32_t directoryEntry) {
    struct directoryEntry entry;
    entry.address = (void *)(directoryEntry & 0xFFFFF000);
    entry.present = directoryEntry & 0x1;
    entry.perms = (page_perms)((directoryEntry >> 1) & 0x1);
    entry.priv = (page_priv)((directoryEntry >> 2) & 0x1);
    entry.write_through = (directoryEntry >> 3) & 0x1;
    entry.cache_disabled = (directoryEntry >> 4) & 0x1;
    // entry.accessed = (directoryEntry >> 5) & 0x1;
    // entry.available = (directoryEntry >> 6) & 0x1;
    return entry;
}

static struct pagetableEntry get_table_entry(uint32_t tableEntry) {
    struct pagetableEntry entry;
    entry.address = (void *)(tableEntry & 0xFFFFF000);
    entry.present = tableEntry & 0x1;
    entry.perms = (page_perms)((tableEntry >> 1) & 0x1);
    entry.priv = (page_priv)((tableEntry >> 2) & 0x1);
    entry.write_through = (tableEntry >> 3) & 0x1;
    entry.cache_disabled = (tableEntry >> 4) & 0x1;
    // entry.accessed = (tableEntry >> 5) & 0x1;
    entry.dirty = (tableEntry >> 6) & 0x1;
    // entry.PAT = (tableEntry >> 7) & 0x1;
    entry.global = (tableEntry >> 8) & 0x1;
    // entry.available = (tableEntry >> 9) & 0x1;
    return entry;
}

static inline void invlpg(void *virtaddr) {
    uint32_t _virtaddr = (uint32_t)virtaddr;
    asm volatile("invlpg (%0)" ::"r"(_virtaddr) : "memory");
}

void paging::init() {
    clearPageTables((void *)0x0, KERNEL_VIRT_ADDRESS / CONFIG_ARCH_PAGE_SIZE);
}

void *paging::get_physaddr(void *virtualaddr) {
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    // TODO: check if entry is actually present
    return (void *)(pagetables[pdindex][ptindex] & 0xFFFFF000);
}

void *paging::get_physaddr_unaligned(void *virtualaddr) {
    uint64_t misalignment = ((uint64_t)virtualaddr) % CONFIG_ARCH_PAGE_SIZE;
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    // TODO: check if entry is actually present
    return (void *)((pagetables[pdindex][ptindex] & 0xFFFFF000) + misalignment);
}

// Both addresses have to be page-aligned!
void paging::map_page(void *physaddr, void *virtualaddr, size_t count, bool massflush, bool global) {

    uint32_t entry = make_table_entry(
        {.address = physaddr, .global = global, .cache_disabled = false, .write_through = false, .priv = USER, .perms = RW, .present = true});

    uint32_t pDirIndex;
    uint32_t pTableIndex;
    bool do_invlpg;

    for (size_t i = 0; i < count; i++) {
        pDirIndex = (uint32_t)virtualaddr >> 22;
        pTableIndex = (uint32_t)virtualaddr >> 12 & 0x03FF;
        do_invlpg = (pagetables[pDirIndex][pTableIndex] & 0x1) && !massflush;

        pagetables[pDirIndex][pTableIndex] = entry;
        if (do_invlpg) {
            invlpg(virtualaddr);
        }

        virtualaddr = ((uint8_t *)virtualaddr) + 4096;
        physaddr = ((uint8_t *)physaddr) + 4096;

        entry = change_table_address(entry, physaddr);
    }

    if (massflush) {
        loadPageDirectory(get_physaddr(page_directory));
    }
}

void paging::clearPageTables(void *virtualaddr, uint32_t pagecount, bool massflush) {
    uint32_t pDirIndex;
    uint32_t pTableIndex;
    bool do_invlpg;

    for (size_t i = 0; i < pagecount; i++) {
        pDirIndex = (uint32_t)virtualaddr >> 22;
        pTableIndex = (uint32_t)virtualaddr >> 12 & 0x03FF;
        do_invlpg = (pagetables[pDirIndex][pTableIndex] & 0x1) && !massflush;

        pagetables[pDirIndex][pTableIndex] = 0;
        if (do_invlpg) {
            invlpg(virtualaddr);
        }
        virtualaddr = ((uint8_t *)virtualaddr) + 4096;
    }

    if (massflush) {
        loadPageDirectory(get_physaddr(page_directory));
    }
}

bool paging::is_readable(const void *virtualaddr) {
    uint32_t pdindex = (uint32_t)virtualaddr >> 22;
    uint32_t ptindex = (uint32_t)virtualaddr >> 12 & 0x03FF;
    return pagetables[pdindex][ptindex] & 0x1;
}

void paging::copyPhysPage(void *dest, void *src) {
    uint32_t before1 = pagetables[0][0];
    uint32_t before2 = pagetables[0][1];
    pagetables[0][0] = make_table_entry(
        {.address = src, .global = false, .cache_disabled = false, .write_through = false, .priv = SUPERVISOR, .perms = RW, .present = true});
    pagetables[0][1] = make_table_entry(
        {.address = dest, .global = false, .cache_disabled = false, .write_through = false, .priv = SUPERVISOR, .perms = RW, .present = true});
    invlpg((void *)0);
    invlpg((void *)CONFIG_ARCH_PAGE_SIZE);
    memcpy((char *)CONFIG_ARCH_PAGE_SIZE, (char *)0, CONFIG_ARCH_PAGE_SIZE);
    pagetables[0][0] = before1;
    pagetables[0][1] = before2;
    invlpg((void *)0);
    invlpg((void *)CONFIG_ARCH_PAGE_SIZE);
}

static unsigned int entry_get_vmm_flags(struct pagetableEntry entry) {
    unsigned int flags = 0;
    if (entry.present) {
        flags |= arch::vmm::FLAGS_PRESENT;
    }
    if (entry.dirty) {
        flags |= arch::vmm::FLAGS_DIRTY;
    }
    if (entry.cache_disabled) {
        flags |= arch::vmm::FLAGS_CACHE_DISABLE;
    }
    if (entry.write_through) {
        flags |= arch::vmm::FLAGS_WRITE_THROUGH;
    }
    if (entry.priv == page_priv::USER) {
        flags |= arch::vmm::FLAGS_USER;
    }
    if (entry.perms == page_perms::R) {
        flags |= arch::vmm::FLAGS_READ_ONLY;
    }
    // IA-32 has no no-execute bit
    return flags;
}

static void entry_set_vmm_flags(struct pagetableEntry *entry, unsigned int flags) {
    entry->present = flags & arch::vmm::FLAGS_PRESENT;
    entry->dirty = flags & arch::vmm::FLAGS_DIRTY;
    entry->cache_disabled = flags & arch::vmm::FLAGS_CACHE_DISABLE;
    entry->write_through = flags & arch::vmm::FLAGS_WRITE_THROUGH;
    entry->priv = flags & arch::vmm::FLAGS_USER ? page_priv::USER : page_priv::SUPERVISOR;
    entry->perms = flags & arch::vmm::FLAGS_READ_ONLY ? page_perms::R : page_perms::RW;
}

uintptr_t arch::vmm::get_page(uintptr_t virt, unsigned int *flags) {
    uint32_t pDirIndex = (uint32_t)virt >> 22;
    uint32_t pTableIndex = (uint32_t)virt >> 12 & 0x03FF;

    struct pagetableEntry entry = get_table_entry(pagetables[pDirIndex][pTableIndex]);

    *flags = entry_get_vmm_flags(entry);
    return (uintptr_t)entry.address;
}

unsigned int arch::vmm::set_page(uintptr_t virt, uintptr_t phys, unsigned int flags) {
    uint32_t pDirIndex = (uint32_t)virt >> 22;
    uint32_t pTableIndex = (uint32_t)virt >> 12 & 0x03FF;

    struct pagetableEntry entry = get_table_entry(pagetables[pDirIndex][pTableIndex]);
    unsigned int flags_old = entry_get_vmm_flags(entry);
    entry = {.address = (void *)phys, .global = false};
    entry_set_vmm_flags(&entry, flags);
    pagetables[pDirIndex][pTableIndex] = make_table_entry(entry);
    return flags_old;
}

void arch::vmm::flush_tlb_single(uintptr_t virt) {
    invlpg((void *)virt);
}

void arch::vmm::flush_tlb_all() {
    loadPageDirectory(paging::get_physaddr(page_directory));
}
