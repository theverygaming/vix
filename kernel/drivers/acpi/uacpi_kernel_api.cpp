#include <vix/config.h>
#include <uacpi/kernel_api.h>
#include <vix/kprintf.h>
#include <vix/macros.h>
#include <vix/mm/mm.h>
#include <vix/arch/common/acpi.h>


uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address) {
    auto s = arch::acpi_get_rsdp();
    if (!s.status().ok()) {
        return UACPI_STATUS_NOT_FOUND;
    }
    *out_rsdp_address = s.value();
    return UACPI_STATUS_OK;
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {
    size_t diff = ALIGN_DOWN_DIFF(addr, CONFIG_ARCH_PAGE_SIZE);
    size_t aligned_size = ALIGN_UP(len + diff, CONFIG_ARCH_PAGE_SIZE);
    void *alloc_addr;
    ASSIGN_OR_PANIC(
        alloc_addr,
        mm::map_arbitrary_phys((mm::paddr_t)addr - diff, aligned_size)
    );
    return (void *)((uintptr_t)alloc_addr + diff);
}

void uacpi_kernel_unmap(void *addr, uacpi_size len) {
    size_t diff = ALIGN_DOWN_DIFF((uintptr_t)addr, CONFIG_ARCH_PAGE_SIZE);
    size_t aligned_size = ALIGN_UP(len + diff, CONFIG_ARCH_PAGE_SIZE);
    mm::unmap_arbitrary_phys((void *)((uintptr_t)addr - diff), aligned_size);
}

void uacpi_kernel_log(uacpi_log_level lvl, const uacpi_char *s) {
    int kp_level = KP_INFO;
    switch(lvl) {
        case UACPI_LOG_DEBUG:
            kp_level = _KP_DEBUG;
            break;
        case UACPI_LOG_TRACE:
            kp_level = _KP_DEBUG;
            break;
        case UACPI_LOG_INFO:
            kp_level = KP_INFO;
            break;
        case UACPI_LOG_WARN:
            kp_level = KP_WARNING;
            break;
        case UACPI_LOG_ERROR:
            kp_level = KP_ERR;
            break;
    }
    // NOTE: the string from uacpi contains a newline already
    kprintf(kp_level, "uacpi: %s", s);
}
