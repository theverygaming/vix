#include <vix/config.h>

#ifdef CONFIG_ARCH_HAS_PAGING
#include <vix/debug.h>
#include <vix/abi/execfmt/elf.h>
#include <vix/fs/vfs.h>
#include <vix/status.h>
#include <vix/abi/execfmt/detect.h>
#include <string.h>
#include <utility>
#include <vix/macros.h>

status::StatusOr<std::pair<arch::vmm::pt_t, void *>> execfmt::load_any(const char *path) {
    RUN_OR_RETURN(
        vfs::open(path),
        {
            auto exec_file = value;
            RUN_OR_RETURN(
                load_any(exec_file),
                {
                    PANIC_IF_ERROR(vfs::close(exec_file));
                    return value;
                },
                {
                    PANIC_IF_ERROR(vfs::close(exec_file));
                }
            );
        },
        {}
    );
}

status::StatusOr<std::pair<arch::vmm::pt_t, void *>> execfmt::load_any(std::shared_ptr<struct vfs::vnode> exec_file) {
    status::StatusOr<void *> (*execfmt_loader)(arch::vmm::pt_t pt, std::shared_ptr<struct vfs::vnode> exec_file) = nullptr;

#if defined(CONFIG_ENABLE_EXECFMT_ELF32) || defined(CONFIG_ENABLE_EXECFMT_ELF64)
    uint8_t buf[5];
    uint8_t elfmagic[4] = {
        0x7f, 'E', 'L', 'F', // ELF magic
    };
    std::pair<uint8_t, status::StatusOr<void *> (*)(arch::vmm::pt_t pt, std::shared_ptr<struct vfs::vnode> exec_file)> elf_dispatch[] = {
#ifdef CONFIG_ENABLE_EXECFMT_ELF32
        {elf::ELFCLASS32, &elf::load_elf32},
#endif
#ifdef CONFIG_ENABLE_EXECFMT_ELF64
        {elf::ELFCLASS64, &elf::load_elf64},
#endif
    };
    CHKSTATUS(
        vfs::read(exec_file, 0, &buf, sizeof(buf)),
        {
            if (value == sizeof(buf) && memcmp(buf, elfmagic, sizeof(elfmagic)) == 0) {
                for (size_t i = 0; i < ARRAY_SIZE(elf_dispatch); i++) {
                    uint8_t elfclass = buf[4];
                    if (elfclass == elf_dispatch[i].first) {
                        DEBUG_PRINTF("execfmt::load_any detected ELF class %u\n", (unsigned int)elfclass);
                        execfmt_loader = elf_dispatch[i].second;   
                    }
                }
            }
        },
        {}
    );
#endif

    if (execfmt_loader == nullptr) {
        return status::StatusCode::EGENERIC;
    }

    arch::vmm::pt_t pt;
    // FIXME: problem! we should probably, maybe not allocate a e.g. level 5 page table on x86_64 with 5-level paging... Maybe add smth like CONFIG_ARCH_USER_PT_LEVEL or smth?
    ASSIGN_OR_PANIC(pt, arch::vmm::alloc_pt(CONFIG_ARCH_PAGING_LEVELS));

    arch::vmm::pt_t prev_pt = arch::vmm::get_active_pt();
    arch::vmm::load_pt(pt);

    auto execfmt_ret = execfmt_loader(pt, exec_file);

    arch::vmm::load_pt(prev_pt);

    if (execfmt_ret.status().ok()) {
        std::pair<arch::vmm::pt_t, void *> rpair = {
            .first = pt,
            .second = execfmt_ret.value()
        };
        return rpair;
    } else {
        arch::vmm::free_pt(pt);
        return execfmt_ret.status().code();
    }
}

#endif // CONFIG_ARCH_HAS_PAGING
