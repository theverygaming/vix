#include <kprintf.h>
#include <panic.h>
#include <types.h>

extern "C" void exception_handler(uint64_t n, uint64_t esr, uint64_t elr, uint64_t spsr, uint64_t far) {
    kprintf(KP_EMERG, "except: ESR: 0x%p ELR: 0x%p SPSR: 0x%p FAR: 0x%p\n", esr, elr, spsr, far);
    if (n < 4) {
        const char *types[] = {"Synchronous", "IRQ", "FIQ", "Serror"};
        kprintf(KP_EMERG, "except: %s\n", types[n]);
    }

    // from ARM DDI 0487I.a page 9267
    switch (esr >> 26) {
    case 0b000000:
        kprintf(KP_EMERG, "except: | Unknown reason\n");
        break;
    case 0b000001:
        kprintf(KP_EMERG, "except: | Trapped WFI or WFE instruction\n");
        break;
    case 0b000011:
        kprintf(KP_EMERG, "except: | Trapped MCR or MRC access with (coproc==0b1111)\n");
        break;
    case 0b000100:
        kprintf(KP_EMERG, "except: | Trapped MCRR or MRRC access with (coproc==0b1111)\n");
        break;
    case 0b000101:
        kprintf(KP_EMERG, "except: | Trapped MCR or MRC access with (coproc==0b1110)\n");
        break;
    case 0b000110:
        kprintf(KP_EMERG, "except: | Trapped LDC or STC access\n");
        break;
    case 0b000111:
        kprintf(KP_EMERG, "except: | Advanced SIMD or floating-point functionality trapped by a HCPTR.{TASE, TCP10} control\n");
        break;
    case 0b001000:
        kprintf(KP_EMERG, "except: | Trapped VMRS access, from ID group traps, that is not reported using EC 0b000111\n");
        break;
    case 0b001100:
        kprintf(KP_EMERG, "except: | Trapped MRRC access with (coproc==0b1110)\n");
        break;
    case 0b001110:
        kprintf(KP_EMERG, "except: | Illegal exception return to AArch32 state\n");
        break;
    case 0b010001:
        kprintf(KP_EMERG, "except: | Exception on SVC execution in AArch32 state routed to EL2\n");
        break;
    case 0b010010:
        kprintf(KP_EMERG, "except: | HVC instruction execution in AArch32 state, when HVC is not disabled\n");
        break;
    case 0b010011:
        kprintf(KP_EMERG, "except: | Trapped execution of SMC instruction in AArch32 state\n");
        break;
    case 0b100000:
        kprintf(KP_EMERG, "except: | Prefetch Abort from a lower Exception level\n");
        break;
    case 0b100001:
        kprintf(KP_EMERG, "except: | Prefetch Abort taken without a change in Exception level\n");
        break;
    case 0b100010:
        kprintf(KP_EMERG, "except: | PC alignment exception\n");
        break;
    case 0b100100:
        kprintf(KP_EMERG, "except: | Data Abort from a lower Exception level\n");
        break;
    case 0b100101:
        kprintf(KP_EMERG, "except: | Data Abort taken without a change in Exception level\n");
        break;
    default:
        kprintf(KP_EMERG, "except: | ?\n");
        break;
    }

    // Data abort -- from ARM DDI 0487I.a page ~9542
    if (esr >> 26 == 0b100100 || esr >> 26 == 0b100101) {
        if (esr & 0x40) {
            kprintf(KP_EMERG, "except: | read\n");
        } else {
            kprintf(KP_EMERG, "except: | write\n");
        }

        if (esr & 0xF0) {
            switch (esr & 0x3F) {
            case 0b010000:
                kprintf(KP_EMERG, "except: | Synchronous External abort, not on translation table walk\n");
                break;
            case 0b010001:
                kprintf(KP_EMERG, "except: | Asynchronous SError interrupt\n");
                break;
            case 0b100001:
                kprintf(KP_EMERG, "except: | Alignment fault\n");
                break;
            case 0b100010:
                kprintf(KP_EMERG, "except: | Debug exception\n");
                break;
            case 0b110000:
                kprintf(KP_EMERG, "except: | TLB conflict abort\n");
                break;
            default:
                kprintf(KP_EMERG, "except: | ?(0x%p)\n", (size_t)esr & 0xFF);
                break;
            }
        } else {
            uint8_t level = esr & 0x3;
            switch ((esr >> 2) & 0x3) {
            case 0:
                kprintf(KP_EMERG, "except: | Address size fault, level %u\n", (size_t)level);
                break;
            case 1:
                kprintf(KP_EMERG, "except: | Translation fault, level %u\n", (size_t)level);
                break;
            case 2:
                kprintf(KP_EMERG, "except: | Access flag fault, level %u\n", (size_t)level);
                break;
            case 3:
                kprintf(KP_EMERG, "except: | Permission fault, level %u\n", (size_t)level);
                break;
            }
        }
    }
    KERNEL_PANIC("exception");
}
