#include <vix/types.h>

namespace mm {
    struct mem_map_entry {
        uint64_t base;
        uint64_t size;
        enum class type_t {
            NONE = 0,
            RAM,          // usable memory
            RECLAIMABLE,  // some reclaimable memory
            RESERVED,     // system reserved
            ACPI_RECLAIM, // reclaimable ACPI memory
            ACPI_NVS,     // ACPI reserved
            UNUSABLE,     // memory with errors etc.
            DISABLED,     // disabled memory
            PERSISTENT,   // non-volatile memory
            UNKNOWN,      // anything else
        } type;
    };

    void set_mem_map(const struct mem_map_entry *in, size_t len);
    void set_mem_map(struct mem_map_entry (*get_entry)(size_t n), size_t len);
    bool memmap_is_usable(mm::mem_map_entry::type_t type);
    const struct mem_map_entry *get_mem_map(size_t index);
    uint64_t mem_map_get_total_usable_bytes();
}
