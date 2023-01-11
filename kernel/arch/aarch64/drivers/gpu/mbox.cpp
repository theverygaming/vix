#include <arch/drivers/gpu/mbox.h>
#include <stdio.h>

#define MMIO_BASE   0x3F000000
#define MBOX_OFFSET 0xB880

#define MBOX_ADR (MMIO_BASE + MBOX_OFFSET)

#define MBOX_READ   ((volatile uint32_t *)(MBOX_ADR + 0x0))
#define MBOX_POLL   ((volatile uint32_t *)(MBOX_ADR + 0x10))
#define MBOX_SENDER ((volatile uint32_t *)(MBOX_ADR + 0x14))
#define MBOX_STATUS ((volatile uint32_t *)(MBOX_ADR + 0x18))
#define MBOX_CONFIG ((volatile uint32_t *)(MBOX_ADR + 0x1C))
#define MBOX_WRITE  ((volatile uint32_t *)(MBOX_ADR + 0x20))

#define MBOX_RESPONSE 0x80000000
#define MBOX_FULL     0x80000000
#define MBOX_EMPTY    0x40000000

bool mbox_call(int channel, volatile void *buf) {
    if ((uintptr_t)buf % 16) {
        return false;
    }
    while (*MBOX_STATUS & MBOX_FULL) {};

    volatile uint32_t *buf_32 = (volatile uint32_t *)buf;

    uint32_t msg = ((uint64_t)buf & ~0xF) | (channel & 0xF);

    *MBOX_WRITE = msg;

    while (true) {
        while (*MBOX_STATUS & MBOX_EMPTY) {}

        if (*MBOX_READ == msg) {
            return buf_32[1] == MBOX_RESPONSE;
        }
    }
}
