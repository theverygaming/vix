#pragma once
#include <vix/status.h>
#include <utility>
#include <vix/arch/common/paging.h>
#include <memory>
#include <vix/fs/vfs.h>
#include <vix/config.h>

namespace execfmt {
#ifdef CONFIG_ARCH_HAS_PAGING
    status::StatusOr<std::pair<arch::vmm::pt_t, void *>> load_any(const char *path);
    status::StatusOr<std::pair<arch::vmm::pt_t, void *>> load_any(std::shared_ptr<struct vfs::vnode> exec_file);
#endif
}
