#pragma once
#include <vix/status.h>
#include <utility>
#include <vix/arch/common/paging.h>
#include <memory>
#include <vix/fs/vfs.h>

namespace execfmt {
    status::StatusOr<std::pair<arch::vmm::pt_t, void *>> load_any(const char *path);
    status::StatusOr<std::pair<arch::vmm::pt_t, void *>> load_any(std::shared_ptr<struct vfs::vnode> exec_file);
}
