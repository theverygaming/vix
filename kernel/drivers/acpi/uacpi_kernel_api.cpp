#include <uacpi/types.h>
#include <vix/sched.h>
#include <uacpi/status.h>
#include <vix/config.h>
#include <uacpi/kernel_api.h>
#include <vix/kprintf.h>
#include <vix/macros.h>
#include <vix/mm/mm.h>
#include <vix/arch/common/acpi.h>
#include <vix/drivers/pci.h>
#include <vix/kernel/io.h>
#include <vix/mm/kheap.h>
#include <vix/interrupts.h>
#include <vix/debug.h>
#include <vix/time.h>
#include <vix/kernel/irq.h>
#include <vix/sync.h>


uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address) {
    auto s = arch::acpi_get_rsdp();
    if (!s.status().ok()) {
        DEBUG_PRINTF("uacpi: RSDP not found\n");
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

uacpi_status uacpi_kernel_pci_device_open(uacpi_pci_address address, uacpi_handle *out_handle) {
    struct pci::pci_dev *dev = pci::pci_dev_open_force(address.segment, address.bus, address.device, address.function);
    if (dev == nullptr) {
        DEBUG_PRINTF("uacpi: PCI dev not found\n");
        return UACPI_STATUS_NOT_FOUND;
    }
    *out_handle = dev;
    return UACPI_STATUS_OK;
}

void uacpi_kernel_pci_device_close(uacpi_handle handle) {
    pci::pci_dev_close((struct pci::pci_dev *)handle);
}

uacpi_status uacpi_kernel_pci_read8(uacpi_handle device, uacpi_size offset, uacpi_u8 *value) {
    struct pci::pci_dev *dev = (struct pci::pci_dev *)device;
    *value = pci::pci_dev_config_read_8(dev, offset);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_read16(uacpi_handle device, uacpi_size offset, uacpi_u16 *value) {
    struct pci::pci_dev *dev = (struct pci::pci_dev *)device;
    *value = pci::pci_dev_config_read_16(dev, offset);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_read32(uacpi_handle device, uacpi_size offset, uacpi_u32 *value) {
    struct pci::pci_dev *dev = (struct pci::pci_dev *)device;
    *value = pci::pci_dev_config_read_32(dev, offset);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write8(uacpi_handle device, uacpi_size offset, uacpi_u8 value) {
    struct pci::pci_dev *dev = (struct pci::pci_dev *)device;
    pci::pci_dev_config_write_8(dev, offset, value);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write16(uacpi_handle device, uacpi_size offset, uacpi_u16 value) {
    struct pci::pci_dev *dev = (struct pci::pci_dev *)device;
    pci::pci_dev_config_write_16(dev, offset, value);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write32(uacpi_handle device, uacpi_size offset, uacpi_u32 value) {
    struct pci::pci_dev *dev = (struct pci::pci_dev *)device;
    pci::pci_dev_config_write_32(dev, offset, value);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len, uacpi_handle *out_handle) {
    *out_handle = (void *)io_pmio_map(base, len);
    return UACPI_STATUS_OK;
}

void uacpi_kernel_io_unmap(uacpi_handle handle) {
    io_handle_t iohandle = (io_handle_t)handle;
    io_unmap(iohandle);
}

uacpi_status uacpi_kernel_io_read8(uacpi_handle handle, uacpi_size offset, uacpi_u8 *out_value) {
    io_handle_t iohandle = (io_handle_t)handle;
    *out_value = ioread8(iohandle, offset);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_read16(uacpi_handle handle, uacpi_size offset, uacpi_u16 *out_value) {
    io_handle_t iohandle = (io_handle_t)handle;
    *out_value = ioread16(iohandle, offset);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_read32(uacpi_handle handle, uacpi_size offset, uacpi_u32 *out_value) {
    io_handle_t iohandle = (io_handle_t)handle;
    *out_value = ioread32(iohandle, offset);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write8(uacpi_handle handle, uacpi_size offset, uacpi_u8 in_value) {
    io_handle_t iohandle = (io_handle_t)handle;
    iowrite8(iohandle, offset, in_value);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write16(uacpi_handle handle, uacpi_size offset, uacpi_u16 in_value) {
    io_handle_t iohandle = (io_handle_t)handle;
    iowrite16(iohandle, offset, in_value);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write32(uacpi_handle handle, uacpi_size offset, uacpi_u32 in_value) {
    io_handle_t iohandle = (io_handle_t)handle;
    iowrite32(iohandle, offset, in_value);
    return UACPI_STATUS_OK;
}

void *uacpi_kernel_alloc(uacpi_size size) {
    void *allocated = mm::kmalloc(size);
    return allocated;
}

void uacpi_kernel_free(void *mem) {
    if (mem != nullptr) {
        mm::kfree(mem);
    }
}

uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot(void) {
    return time::ns_since_bootup;
}

void uacpi_kernel_stall(uacpi_u8 usec) {
    // FIXME: broken!
}

void uacpi_kernel_sleep(uacpi_u64 msec) {
    // FIXME: broken!
}

uacpi_handle uacpi_kernel_create_mutex(void) {
    // FIXME: broken!
    return mm::kmalloc(1);
}

void uacpi_kernel_free_mutex(uacpi_handle handle) {
    // FIXME: broken!
    mm::kfree(handle);
}

uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle handle, uacpi_u16 timeout_ms) {
    // FIXME: broken!
    return UACPI_STATUS_OK;
}

void uacpi_kernel_release_mutex(uacpi_handle handle) {
    // FIXME: broken!
}

uacpi_handle uacpi_kernel_create_spinlock(void) {
    return spinlock_alloc();
}

void uacpi_kernel_free_spinlock(uacpi_handle handle) {
    spinlock_free((spinlock_t *)handle);
}

uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle handle) {
    spinlock_lock_intdisable((spinlock_t *)handle);
    return 0;
}

void uacpi_kernel_unlock_spinlock(uacpi_handle handle, uacpi_cpu_flags) {
    spinlock_unlock_intdisable((spinlock_t *)handle);
}

uacpi_handle uacpi_kernel_create_event(void) {
    // FIXME: needs a proper implementation!
    return mm::kmalloc(1);
}

void uacpi_kernel_free_event(uacpi_handle handle) {
    // FIXME: needs a proper implementation!
    mm::kfree(handle);
}

uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle handle, uacpi_u16 timeout_ms) {
    // FIXME: needs a proper implementation!
    volatile uint8_t *ev = (volatile uint8_t *)handle;
    uint64_t tstart = time::ns_since_bootup;
    while (time::ns_since_bootup - tstart < 1000000 * timeout_ms) {
        if (*ev) {
            return UACPI_TRUE;
        }
        sched::yield();
    }
    if (*ev) {
        return UACPI_TRUE;
    }
    return UACPI_FALSE;
}

void uacpi_kernel_signal_event(uacpi_handle handle) {
    // FIXME: needs a proper implementation!
    volatile uint8_t *ev = (volatile uint8_t *)handle;
    *ev = 1;
}

void uacpi_kernel_reset_event(uacpi_handle handle) {
    // FIXME: needs a proper implementation!
    volatile uint8_t *ev = (volatile uint8_t *)handle;
    *ev = 0;
}

uacpi_thread_id uacpi_kernel_get_thread_id(void) {
    return (uacpi_thread_id)(uint64_t)sched::mythread()->tid;
}

uacpi_interrupt_state uacpi_kernel_disable_interrupts(void) {
    push_interrupt_disable();
    return 0;
}

void uacpi_kernel_restore_interrupts(uacpi_interrupt_state state) {
    pop_interrupt_disable();
}

uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request*) {
    // FIXME: broken!
    return UACPI_STATUS_NO_HANDLER;
}

struct irq_handler_pass {
    uacpi_interrupt_handler handler;
    uacpi_handle ctx;
    void *irq_handler;
};

static bool irq_handler(void *pv) {
    DEBUG_PRINTF("uacpi: got IRQ!!\n");
    struct irq_handler_pass *p = (struct irq_handler_pass *)pv;
    if (p->handler(p->ctx) == UACPI_INTERRUPT_HANDLED) {
        return true;
    } else {
        return false;
    }
}

uacpi_status uacpi_kernel_install_interrupt_handler(uacpi_u32 irq, uacpi_interrupt_handler handler, uacpi_handle ctx, uacpi_handle *out_irq_handle) {
    struct irq_handler_pass *p = new irq_handler_pass {
        .handler = handler,
        .ctx = ctx,
        .irq_handler = nullptr,
    };
    p->irq_handler = irq::register_irq_handler(irq_handler, irq, p);
    *out_irq_handle = p;
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_uninstall_interrupt_handler(uacpi_interrupt_handler hander, uacpi_handle irq_handle) {
    struct irq_handler_pass *p = (struct irq_handler_pass *)irq_handle;
    irq::deregister_irq_handler(p->irq_handler);
    delete p;
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_schedule_work(
    uacpi_work_type, uacpi_work_handler worker, uacpi_handle ctx
) {
    DEBUG_PRINTF("uacpi: scheduled work\n");
    // TODO: handle uacpi_work_type (not really needed at time of writing since we don't even have SMP)
    sched::start_kworker(worker, ctx);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_wait_for_work_completion(void) {
    // FIXME: broken!
    return UACPI_STATUS_OK;
}
