include Makefile.version

BUILD_SUBDIRS := kernel drivers fs net tests cppstd mm

ifndef ARCH
$(error must have ARCH environment variable set)
endif

SRCARCH := $(ARCH)
export SRCARCH ARCH

-include .config
BUILD_ARCH := $(patsubst "%",%,$(CONFIG_ARCH))
export SRCARCH ARCH

include scripts/Makefile.build

ifdef BUILD_ARCH
BUILD_SUBDIRS += arch/$(BUILD_ARCH)
include arch/$(BUILD_ARCH)/Makefile.arch
endif

.PHONY: all
all: genconfig

genconfig:
	@$(MAKE) --quiet -C scripts/kconfig/ config
	@scripts/kconfig/config Kconfig --silent --syncconfig
	@$(MAKE) kernel

help:
	@echo 'Cleaning targets:'
	@echo '  clean           - remove everything not required for building modules'
	@echo '  proper          - clean + remove everything generated'
	@echo 'Configuration targets:'
	@$(MAKE) --quiet -C scripts/kconfig/ help
	@echo ''
	@echo 'Execute "make" or "make all" to build all targets marked with [*]'

# kconfig targets
config:
	@$(MAKE) --quiet -C scripts/kconfig/ config
	@scripts/kconfig/config Kconfig --oldaskconfig

nconfig: 
	@$(MAKE) --quiet -C scripts/kconfig/ nconfig
	@scripts/kconfig/nconfig Kconfig

menuconfig: 
	@$(MAKE) --quiet -C scripts/kconfig/ menuconfig
	@scripts/kconfig/menuconfig Kconfig

xconfig: 
	@$(MAKE) --quiet -C scripts/kconfig/ xconfig
	@scripts/kconfig/xconfig Kconfig

gconfig: 
	@$(MAKE) --quiet -C scripts/kconfig/ gconfig
	@scripts/kconfig/gconfig Kconfig

alldefconfig: 
	@$(MAKE) --quiet -C scripts/kconfig/ config
	@scripts/kconfig/config Kconfig --silent --alldefconfig

# clean targets
clean:
	@rm -f kernel.o kernel.bin symtab.s symtab.os kernel_partial.o
	@find ./arch ./drivers ./fs ./include ./kernel ./net ./mm ./cppstd ./tests \( -name '*.oasm' -o -name '*.onasm' -o -name '*.ocpp' -o -name '*.ors' -o -name '*.rlib' -o -name '*.o' -o -name '*.os' \) -type f -delete

proper: clean
	@$(MAKE) --quiet -C scripts/kconfig clean
	@rm -f .config .config_old

# compiler stuff

CXX      = $(CROSS_COMPILE)g++
LD       = $(CROSS_COMPILE)ld
NM       = $(CROSS_COMPILE)nm
OBJCOPY  = $(CROSS_COMPILE)objcopy
export CXX LD NM OBJCOPY CXXFLAGS LDFLAGS

# recursive stuff - most of this is based on https://stackoverflow.com/q/56001646/20742078
OBJS     :=
EXTRATARGETS :=
objtree  := .
srctree  := .
define build_subdir
objtree := $$(objtree)/$(1)
srctree := $$(srctree)/$(1)

obj-y :=
obj-d :=
extratargets-y :=

include $$(srctree)/Makefile

OBJS += $$(patsubst %,$$(objtree)/%,$$(obj-y))
EXTRATARGETS += $$(patsubst %,$$(objtree)/%,$$(extratargets-y))

$$(foreach subdir,$$(obj-d),$$(eval $$(call build_subdir,$$(subdir))))

srctree := $$(patsubst %/$(1),%,$$(srctree))
objtree := $$(patsubst %/$(1),%,$$(objtree))
endef

$(foreach dir, $(BUILD_SUBDIRS), $(eval $(call build_subdir,$(dir))))


# final link
.PHONY: kernel
kernel: $(EXTRATARGETS) $(OBJS)
	@$(LD) $(LDFLAGS) -r $(OBJS) -o kernel_partial.o
	@./arch/$(BUILD_ARCH)/archlink.sh
