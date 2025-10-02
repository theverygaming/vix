obj-$(CONFIG_ENABLE_ACPI) := $(patsubst %.c,%.oc,$(subst $(srctree)/,,$(wildcard $(srctree)/uacpi/source/*.c)))
obj-$(CONFIG_ENABLE_ACPI) += uacpi_interface.o

# TODO: allow kernel makefiles to specify extra CFLAGS, include paths (goes with CFLAGS ig) and stuff..

# atm this is stuck on us being unable to configure uacpi because we can't provide any cflags from this makefile
# and the kernel makefiles probably need a significant amount of rewriting to support that lol
