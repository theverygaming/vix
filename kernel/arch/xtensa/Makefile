obj-$(CONFIG_ENABLE_STACKPROTECTOR) += stackprotector.o

obj-y += generic/cpu.o generic/devices.o
obj-y += archinit.o sched_s.o sched.o io.o

obj-$(CONFIG_XTENSA_PLATFORM_ESP8266) += vectors.o vectors_s.o

ifneq ($(strip $(CONFIG_XTENSA_VARIANT_NAME)),)
obj-d += variants/$(CONFIG_XTENSA_VARIANT_NAME)
endif

ifneq ($(strip $(CONFIG_XTENSA_PLATFORM_NAME)),)
obj-d += platforms/$(CONFIG_XTENSA_PLATFORM_NAME)
endif
