#include "idt.h"
#include "paging.h"
#include <stdint.h>

#define FLAG_SET(x, flag) x |= (flag)
#define FLAG_UNSET(x, flag) x &= ~(flag)

typedef struct {
    uint16_t BaseLow;
    uint16_t SegmentSelector;
    uint8_t Reserved;
    uint8_t Flags;
    uint16_t BaseHigh;
} __attribute__((packed)) IDTEntry;

typedef struct {
    uint16_t Limit;
    IDTEntry *Ptr;
} __attribute__((packed)) IDTDescriptor;

IDTEntry g_IDT[256];

IDTDescriptor g_IDTDescriptor = {sizeof(g_IDT) - 1, g_IDT};

extern "C" void i686_IDT_Load(IDTDescriptor *idtDescriptor);

void idt::i686_IDT_SetGate(int interrupt, void *base, uint16_t segmentDescriptor, uint8_t flags) {
    g_IDT[interrupt].BaseLow = ((uint32_t)base) & 0xFFFF;
    g_IDT[interrupt].SegmentSelector = segmentDescriptor;
    g_IDT[interrupt].Reserved = 0;
    g_IDT[interrupt].Flags = flags;
    g_IDT[interrupt].BaseHigh = ((uint32_t)base >> 16) & 0xFFFF;
}

void idt::i686_IDT_EnableGate(int interrupt) {
    FLAG_SET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

void idt::i686_IDT_DisableGate(int interrupt) {
    FLAG_UNSET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

void idt::i686_IDT_Initialize() {
    i686_IDT_Load(&g_IDTDescriptor);
}