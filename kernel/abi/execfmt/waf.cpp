#include <vix/debug.h>
#include <vix/macros.h>
#include <vix/abi/execfmt/waf.h>
#include <string.h>

struct __attribute__((packed)) waf_header {
    char magic[4];
    size_t memsize;
    size_t entry_offset;
};

status::StatusOr<void *> execfmt::waf::load_waf(mm::mem_areas_t *areas, std::shared_ptr<struct vfs::vnode> file) {
    struct waf_header header;

    RUN_OR_RETURN(
        vfs::read(file, 0, &header, sizeof(header)),
        {
            if (value != sizeof(header)) {
                return status::StatusCode::EGENERIC;
            }
        },
        {}
    );

    size_t codeoffset = sizeof(struct waf_header);
    size_t codesize = file->attrs.sz_bytes - codeoffset;

    DEBUG_PRINTF("WAF! code size: %u bytes; memory size: %u bytes; entry offset: 0x%p\n", codesize, header.memsize, header.entry_offset);

    if (header.entry_offset >= header.memsize) {
        DEBUG_PRINTF("entry outside of application memory");
        return status::StatusCode::EGENERIC;
    }

    if (codesize > header.memsize) {
        DEBUG_PRINTF("code size larger than memory size");
        return status::StatusCode::EGENERIC;
    }

    size_t memsize_real = ALIGN_UP(header.memsize, CONFIG_ARCH_PAGE_SIZE);

    void *execmem;
    ASSIGN_OR_RETURN(execmem, mm::mem_areas_alloc_range(
        areas,
        memsize_real,
        {
            .user = true,
            .read_only = false,
            .no_execute = false,
            .cache = mm::caching_type::WRITE_BACK,
        }
        // FIXME: should probably specify the arange here, especially on mmu-capable architectures this is important.
    ));

    memset(execmem, 0, memsize_real);

    RUN_OR_RETURN(
        vfs::read(file, codeoffset, execmem, codesize),
        {
            if (value != codesize) {
                return status::StatusCode::EGENERIC;
            }
        },
        {}
    );

    return (void *)((uint8_t *)execmem + header.entry_offset);
}
