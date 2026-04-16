#include <forward_list>
#include <vix/config.h>
#include <vix/mm/mm.h>
#include <vix/kernel/io.h>
#include <vix/initfn.h>
#include <vix/drivers/pci.h>
#include <vix/kernel/irq.h>
#include <vix/module.h>
#include <vix/arch/common/paging.h>
#include <vix/debug.h>

MODULE_AUTHOR("theverygaming");
MODULE_DESCRIPTION("ac97");
MODULE_VERSION("0.0.1");

#define NABM_GLOBAL_CONTROL 0x2C
#define NABM_PCM_OUT_BOX_BASE 0x10
#define NABM_BOX_BUFADDR 0x00

#define NAM_RESET 0x00
#define NAM_PCM_OUTPUT 0x18
#define NAM_MASTER_OUTPUT_VOLUME 0x02

struct ac97_card {
    // IO stuff
    struct pci::pci_dev *pcidev;
    io_handle_t bar0;
    io_handle_t bar1;
    void *irq_handle;

    // internal state
    uint8_t *bufferptr;
};

std::forward_list<struct ac97_card *> ac97_instances;


static mm::paddr_t get_physaddr_unaligned(mm::vaddr_t virt) {
    size_t diff = ALIGN_DOWN_DIFF(virt, CONFIG_ARCH_PAGE_SIZE);
    mm::paddr_t addr = arch::vmm::get_page(ALIGN_DOWN(virt, CONFIG_ARCH_PAGE_SIZE), nullptr);
    return addr + diff;
}

static void ac97_irq(struct ac97_card *ctx) {
    DEBUG_PRINTF("ac97 IRQ\n");

    // clear all 3 interrupt status bits (// TODO: figure out what exactly 0x16 is lol osdev wiki kinda lacking here - NOTE: figured it out. It's part of the register box. // TODO: write macros for register box)
    iowrite16(ctx->bar1, 0x16, 0b111 << 2);

    // this probably fired cuz the card loaded a BDL entry, // set last valid BDL entry (// TODO: figure out what exactly 0x15 is lol osdev wiki kinda lacking here)
    uint8_t current = ioread8(ctx->bar1, 0x15);
    DEBUG_PRINTF("ac97 loaded BDL %u\n", (unsigned int)current);
    iowrite8(ctx->bar1, 0x15, (current + 1) % 32);
}

static bool irq_handler(void *ictx) {
    ac97_card *ctx = (ac97_card *)ictx;
    ac97_irq(ctx);
    return true;
}

struct __attribute__((packed)) bdl_entry {
    uint32_t buf_paddr;
    uint16_t n_samples;
    uint16_t flags;
};

static void ac97_init(struct ac97_card *ctx) {
    // https://wiki.osdev.org/AC97
    pci::pci_dev_master(ctx->pcidev, true);

    int irqline = pci::pci_dev_get_irqline(ctx->pcidev);
    DEBUG_PRINTF("ac97 irq: %u\n", (uint32_t)irqline);

    ctx->irq_handle = irq::register_irq_handler(irq_handler, irqline, ctx);

    // enable interrupts, cold reset
    iowrite32(ctx->bar1, NABM_GLOBAL_CONTROL, (1 << 0) | (1 << 1));

    // reset NAM registers
    iowrite16(ctx->bar0, NAM_RESET, 1);

    // set PCM volume to max
    iowrite16(ctx->bar0, NAM_PCM_OUTPUT, 0);

    // set Master volume to max
    iowrite16(ctx->bar0, NAM_MASTER_OUTPUT_VOLUME, 0);

    // set up buffer, and write some example data
    size_t bufsize = 0xFFFF;
    void *tmp_alloc;
    ASSIGN_OR_PANIC(tmp_alloc, mm::allocate_contiguous(bufsize)); // TODO: free
    ctx->bufferptr = (uint8_t *)tmp_alloc;
    size_t buf_audio_entries = (bufsize - ((sizeof(struct bdl_entry) * 32))) / sizeof(int16_t);
    // soundcard needs it even
    if (buf_audio_entries & 1) {
        buf_audio_entries -= 1;
    }
    int16_t *buf_audio = (int16_t *)(ctx->bufferptr + (sizeof(struct bdl_entry) * 32));
    struct bdl_entry *bdl = (struct bdl_entry *)tmp_alloc;
    for (int i = 0; i < 32; i++) {
        bdl[i] = {
            .buf_paddr = (uint32_t)get_physaddr_unaligned((mm::vaddr_t)buf_audio),
            .n_samples = (uint16_t)buf_audio_entries,
            // interrupt on transfer
            .flags = 1 << 15,
        };
    }
    for (size_t i = 0; i < buf_audio_entries; i++) {
        buf_audio[i] = (i % 200) > 100 ? -32768 : 32767;
    }

    // reset DMA transfer control (// TODO: figure out what exactly 0x1B is lol osdev wiki kinda lacking here - NOTE: figured it out. It's part of the register box. // TODO: write macros for register box)
    iowrite8(ctx->bar1, 0x1B, 1 << 1);
    while (ioread8(ctx->bar1, 0x1B) & (1 << 1)) {
        DEBUG_PRINTF("waiting for DMA transfer control reset...\n");
    }

    // set up PCM OUT NABM register box
    iowrite32(ctx->bar1, NABM_PCM_OUT_BOX_BASE + NABM_BOX_BUFADDR, get_physaddr_unaligned((mm::vaddr_t)ctx->bufferptr));

    // set last valid BDL entry (// TODO: figure out what exactly 0x15 is lol osdev wiki kinda lacking here)
    iowrite8(ctx->bar1, 0x15, 0);

    // set DMA transfer control to transfer data and Last Buffer Entry Interrupt and IOC interrupt enable (// TODO: figure out what exactly 0x1B is lol osdev wiki kinda lacking here - NOTE: figured it out. It's part of the register box. // TODO: write macros for register box)
    iowrite8(ctx->bar1, 0x1B, (1 << 0) | (1 << 2) | (1 << 4));

    DEBUG_PRINTF("ac97 init finished!\n");
}

static bool ac97_probe(struct pci::pci_dev *dev) {
    struct ac97_card *card = new ac97_card {
        .pcidev = dev,
        .bar0 = pci_bar_iomap(pci::pci_dev_get_bar(dev, 0)),
        .bar1 = pci_bar_iomap(pci::pci_dev_get_bar(dev, 1)),
        .irq_handle = nullptr,
        .bufferptr = nullptr,
    };
    ac97_instances.push_front(card);

    ac97_init(card);

    return true;
}

struct pci::pci_driver ac97_pci = {
    .name = "ac97",
    .vendor_id = 0x8086,
    .device_id = 0x2415,
    .probe = ac97_probe,
};

static void ac97_driver_init() {
    pci::register_driver(&ac97_pci);
}

static int init() {
    kprintf(KP_INFO, "ac97: loaded\n");
    ac97_driver_init();
    return 0;
}

static void exit() {
    kprintf(KP_INFO, "ac97: unloaded\n");
}

MODULE_INIT(init);
MODULE_EXIT(exit);

// HACK: initfn when the module is in-tree
INITFN_DEFINE(ac97_driver_init, INITFN_DRIVER_INIT, 0, ac97_driver_init);
