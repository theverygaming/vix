#pragma once
#include <vix/mm/mm.h>
#include <vix/status.h>

namespace arch {
    // Returns the physical address to either the ACPI 1.0 RSDP or (preferrably) the ACPI 2.0 RSDP or an error.
    status::StatusOr<mm::paddr_t> acpi_get_rsdp();
}
