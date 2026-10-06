#include <vix/abi/linux/linux.h>
#include <vix/abi/execfmt/detect.h>
#include <vix/status.h>
#include <vix/abi/linux/arch.h>

status::StatusOr<sched::tid_t> abi::linux::exec(const char *path, std::vector<std::string> *args) {
    std::pair<::arch::vmm::pt_t, void *> exec;
    ASSIGN_OR_RETURN(exec, execfmt::load_any(path));

    auto status = abi::linux::arch::start_process(exec.first, exec.second, args);
    if (!status.status().ok()) {
        ::arch::vmm::free_pt(exec.first);
        return status.status().code();
    } else {
        return status.value();
    }
}
