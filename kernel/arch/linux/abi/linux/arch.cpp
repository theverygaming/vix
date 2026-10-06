#include <vix/status.h>
#include <vix/abi/linux/arch.h>

status::StatusOr<sched::tid_t> abi::linux::arch::start_process(::arch::vmm::pt_t pt, void *entrypoint, std::vector<std::string> *args) {
    return status::StatusCode::EGENERIC;
}
