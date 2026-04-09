#include <vix/panic.h>
#include <uacpi/log.h>
#include <vix/initfn.h>
#include <uacpi/uacpi.h>
#include <uacpi/event.h>
#include <vix/kprintf.h>
#include <vix/mm/kheap.h>
#include <uacpi/context.h>
#include <uacpi/sleep.h>
#include <vix/drivers/acpi.h>
#include <vix/interrupts.h>


void acpi::reboot() {
    // epicly stolen from astral source; apparently some HW expects S5 prep before rebooting
    uacpi_status ret = uacpi_prepare_for_sleep_state(UACPI_SLEEP_STATE_S5);
    if (uacpi_unlikely_error(ret)) {
        kprintf(KP_ERR, "uacpi reboot: uacpi_prepare_for_sleep_state error: %s\n", uacpi_status_to_string(ret));
        return;
    }
    push_interrupt_disable();
    ret = uacpi_reboot();
    if (uacpi_unlikely_error(ret)) {
        kprintf(KP_ERR, "uacpi reboot: uacpi_reboot error: %s\n", uacpi_status_to_string(ret));
        pop_interrupt_disable();
        return;
    }
    KERNEL_PANIC("reboot");
}

void acpi::shutdown() {
    uacpi_status ret = uacpi_prepare_for_sleep_state(UACPI_SLEEP_STATE_S5);
    if (uacpi_unlikely_error(ret)) {
        kprintf(KP_ERR, "uacpi shutdown: uacpi_prepare_for_sleep_state error: %s\n", uacpi_status_to_string(ret));
        return;
    }
    push_interrupt_disable();
    ret = uacpi_enter_sleep_state(UACPI_SLEEP_STATE_S5);
    if (uacpi_unlikely_error(ret)) {
        kprintf(KP_ERR, "uacpi shutdown: uacpi_enter_sleep_state error: %s\n", uacpi_status_to_string(ret));
        pop_interrupt_disable();
        return;
    }
    KERNEL_PANIC("shutdown");
}

static void uacpi_init() {
    uacpi_context_set_log_level(UACPI_LOG_TRACE);
    uacpi_status ret = uacpi_setup_early_table_access(mm::kmalloc(1000000), 1000000);
    if (uacpi_unlikely_error(ret)) {
        kprintf(KP_ERR, "uacpi init: uacpi_setup_early_table_access error: %s\n", uacpi_status_to_string(ret));
        return;
    }

    ret = uacpi_initialize(0);
    if (uacpi_unlikely_error(ret)) {
        kprintf(KP_ERR, "uacpi init: uacpi_initialize error: %s\n", uacpi_status_to_string(ret));
        return;
    }

    ret = uacpi_namespace_load();
    if (uacpi_unlikely_error(ret)) {
        kprintf(KP_ERR, "uacpi init: uacpi_namespace_load error: %s\n", uacpi_status_to_string(ret));
        return;
    }

    ret = uacpi_namespace_initialize();
    if (uacpi_unlikely_error(ret)) {
        kprintf(KP_ERR, "uacpi init: uacpi_namespace_initialize error: %s\n", uacpi_status_to_string(ret));
        return;
    }

    ret = uacpi_finalize_gpe_initialization();
    if (uacpi_unlikely_error(ret)) {
        kprintf(KP_ERR, "uacpi init: uacpi_finalize_gpe_initialization error: %s\n", uacpi_status_to_string(ret));
        return;
    }

    kprintf(KP_INFO, "uacpi init: done!\n");
}

INITFN_DEFINE(uacpi_init, INITFN_DRIVER_INIT, 0, uacpi_init);
