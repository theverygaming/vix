# TODO: improve the way we can set flags in general. Requires significant refactor of core Makefiles
ifeq ($(CONFIG_ENABLE_ACPI),y)
# override INT_CFLAGS += -DUACPI_BAREBONES_MODE
endif
obj-$(CONFIG_ENABLE_ACPI) := $(patsubst %.c,%.o,$(subst $(srctree)/,,$(wildcard $(srctree)/uacpi/source/*.c)))
obj-$(CONFIG_ENABLE_ACPI) += uacpi_interface.o uacpi_kernel_api.o
