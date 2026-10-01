obj-y += kheap.o pmm.o memmap.o memtest.o slab.o mm.o
obj-$(CONFIG_ARCH_HAS_PAGING) += vmm.o

ifeq ($(CONFIG_ARCH_X86),y)
obj-$(CONFIG_ARCH_HAS_PAGING) += pgtable.o
endif
