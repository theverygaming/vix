#include <vix/config.h>
#include <vix/debug.h>
#include <vix/abi/execfmt/elf.h>
#include <vix/abi/execfmt/waf.h>
#include <vix/fs/vfs.h>
#include <vix/status.h>
#include <vix/abi/execfmt/detect.h>
#include <string.h>
#include <utility>
#include <vix/macros.h>
#include <vix/mm/mm.h>

status::StatusOr<std::pair<mm::mem_areas_t, void *>> execfmt::load_any(const char *path) {
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

status::StatusOr<std::pair<mm::mem_areas_t, void *>> execfmt::load_any(std::shared_ptr<struct vfs::vnode> exec_file) {
    status::StatusOr<void *> (*execfmt_loader)(mm::mem_areas_t *areas, std::shared_ptr<struct vfs::vnode> exec_file) = nullptr;

#if defined(CONFIG_ENABLE_EXECFMT_ELF32) || defined(CONFIG_ENABLE_EXECFMT_ELF64)
    do {
        uint8_t buf[5];
        uint8_t elfmagic[4] = {
            0x7f, 'E', 'L', 'F', // ELF magic
        };
        std::pair<uint8_t, status::StatusOr<void *> (*)(mm::mem_areas_t *areas, std::shared_ptr<struct vfs::vnode> exec_file)> elf_dispatch[] = {
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
    } while(0);
#endif
#ifdef CONFIG_ENABLE_EXECFMT_WAF
    do {
        uint8_t buf[4];
        uint8_t wafmagic[4] = {
            'W', 'A', 'F', '!',
        };
        CHKSTATUS(
            vfs::read(exec_file, 0, &buf, sizeof(buf)),
            {
                if (value == sizeof(buf) && memcmp(buf, wafmagic, sizeof(wafmagic)) == 0) {
                    execfmt_loader = &waf::load_waf;
                }
            },
            {}
        );
    } while(0);
#endif

    if (execfmt_loader == nullptr) {
        return status::StatusCode::EGENERIC;
    }

    mm::mem_areas_t areas;
    PANIC_IF_ERROR(mm::mem_areas_init(&areas));

    void *token = mm::mem_areas_tmp_load(&areas);

    auto execfmt_ret = execfmt_loader(&areas, exec_file);

    mm::mem_areas_tmp_restore(token);

    if (execfmt_ret.status().ok()) {
        std::pair<mm::mem_areas_t, void *> rpair = {
            .first = areas,
            .second = execfmt_ret.value()
        };
        return rpair;
    } else {
        mm::mem_areas_deinit(&areas);
        return execfmt_ret.status().code();
    }
}
