#include <vix/debug.h>
#include <vix/kprintf.h>
#include <vix/arch/common/cpu.h>

extern "C" void trap_handler(struct arch::full_ctx *ctx) {
    DEBUG_PRINTF("C++ trap handler! rip: 0x%p rax: 0x%p\n", ctx->rip, ctx->rax);
}
