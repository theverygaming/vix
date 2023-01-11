#include <panic.h>
#include <types.h>

extern "C" void exception_handler(uint64_t n, uint64_t esr, uint64_t elr, uint64_t spsr, uint64_t far) {
    printf("ESR: 0x%p ELR: 0x%p SPSR: 0x%p FAR: 0x%p\n", esr, elr, spsr, far);
    if (n < 4) {
        const char *types[] = {"Synchronous", "IRQ", "FIQ", "Serror"};
        printf("%s", types[n]);
    }

    // from ARM DDI 0487I.a page 9267
    switch (esr >> 26) {
    case 0b000000:
        printf(" | Unknown reason");
        break;
    case 0b000001:
        printf(" | Trapped WFI or WFE instruction");
        break;
    case 0b000011:
        printf(" | Trapped MCR or MRC access with (coproc==0b1111)");
        break;
    case 0b000100:
        printf(" | Trapped MCRR or MRRC access with (coproc==0b1111)");
        break;
    case 0b000101:
        printf(" | Trapped MCR or MRC access with (coproc==0b1110)");
        break;
    case 0b000110:
        printf(" | Trapped LDC or STC access");
        break;
    case 0b000111:
        printf(" | Advanced SIMD or floating-point functionality trapped by a HCPTR.{TASE, TCP10} control");
        break;
    case 0b001000:
        printf(" | Trapped VMRS access, from ID group traps, that is not reported using EC 0b000111");
        break;
    case 0b001100:
        printf(" | Trapped MRRC access with (coproc==0b1110)");
        break;
    case 0b001110:
        printf(" | Illegal exception return to AArch32 state");
        break;
    case 0b010001:
        printf(" | Exception on SVC execution in AArch32 state routed to EL2");
        break;
    case 0b010010:
        printf(" | HVC instruction execution in AArch32 state, when HVC is not disabled");
        break;
    case 0b010011:
        printf(" | Trapped execution of SMC instruction in AArch32 state");
        break;
    case 0b100000:
        printf(" | Prefetch Abort from a lower Exception level");
        break;
    case 0b100001:
        printf(" | Prefetch Abort taken without a change in Exception level");
        break;
    case 0b100010:
        printf(" | PC alignment exception");
        break;
    case 0b100100:
        printf(" | Data Abort from a lower Exception level");
        break;
    case 0b100101:
        printf(" | Data Abort taken without a change in Exception level");
        break;
    default:
        printf(" | ?");
        break;
    }

    // Data abort -- from ARM DDI 0487I.a page ~9542
    if (esr >> 26 == 0b100100 || esr >> 26 == 0b100101) {
        if (esr & 0x40) {
            printf(" | read");
        } else {
            printf(" | write");
        }

        if (esr & 0xF0) {
            switch (esr & 0x3F) {
            case 0b010000:
                printf(" | Synchronous External abort, not on translation table walk");
                break;
            case 0b010001:
                printf(" | Asynchronous SError interrupt");
                break;
            case 0b100001:
                printf(" | Alignment fault");
                break;
            case 0b100010:
                printf(" | Debug exception");
                break;
            case 0b110000:
                printf(" | TLB conflict abort");
                break;
            default:
                printf(" | ?(0x%p)", (size_t)esr & 0xFF);
                break;
            }
        } else {
            uint8_t level = esr & 0x3;
            switch ((esr >> 2) & 0x3) {
            case 0:
                printf(" | Address size fault, level %u", (size_t)level);
                break;
            case 1:
                printf(" | Translation fault, level %u", (size_t)level);
                break;
            case 2:
                printf(" | Access flag fault, level %u", (size_t)level);
                break;
            case 3:
                printf(" | Permission fault, level %u", (size_t)level);
                break;
            }
        }
    }
    printf("\n");
    KERNEL_PANIC("exception");
}
