#pragma once
#include <vix/mm/mm.h>
#include <vix/fs/vfs.h>
#include <memory>
#include <utility>
#include <vix/status.h>
#include <vix/types.h>
#include <vix/config.h>

#ifdef CONFIG_ENABLE_EXECFMT_WAF
namespace execfmt::waf {
    /*
     * Returns entry point on success.
     * Assumes the mem area is already initialized and in case of failure will be destroyed including the allocations contained within.
     */
    status::StatusOr<void *> load_waf(mm::mem_areas_t *areas, std::shared_ptr<struct vfs::vnode> file);
}
#endif
