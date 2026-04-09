#include <vix/initfn.h>
#include <uacpi/uacpi.h>
#include <uacpi/event.h>
#include <vix/kprintf.h>
#include <vix/mm/kheap.h>


static void uacpi_init() {
    uacpi_status ret = uacpi_setup_early_table_access(mm::kmalloc(1000), 1000);
    if (uacpi_unlikely_error(ret)) {
        kprintf(KP_ERR, "uacpi: uacpi_setup_early_table_access error: %s\n", uacpi_status_to_string(ret));
    }
}

INITFN_DEFINE(uacpi_init, INITFN_DRIVER_INIT, 0, uacpi_init);
