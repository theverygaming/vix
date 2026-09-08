#pragma once
#include <vix/types.h>
#include <vix/config.h>

namespace arch {
    struct __attribute__((packed)) full_ctx {
#ifdef CONFIG_XTENSA_PLATFORM_ESP8266
        uint32_t aregs[16];
        uint32_t pc;
        uint32_t sar;
        uint32_t ps;

        // FIXME: we are probably missing some special regs here. We should have some way to specify xtensa options or smth... Probably kconfig stuff..

        // these won't be restored, only saved on entry
        uint32_t exccause;
        uint32_t excvaddr;
        uint32_t debugcause;
#endif
    };

    struct __attribute__((packed)) ctx {
#ifdef CONFIG_XTENSA_PLATFORM_ESP8266
        uint32_t a0;
        uint32_t a12;
        uint32_t a13;
        uint32_t a14;
        uint32_t a15;
#endif
    };
}
