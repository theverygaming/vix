# erase defaults
CXX=
LD=

include Makefile.version

BUILD_SUBDIRS := kernel drivers fs net tests cppstd mm

-include .config
BUILD_ARCH := $(patsubst "%",%,$(CONFIG_ARCH))

define getcfgvar
ifdef CONFIG_$(1)
ifeq ($($(1)),)
$(1) = $(subst $\",,$(CONFIG_$(1)))
endif
endif
endef

$(eval $(call getcfgvar,ARCH))
ifdef M
$(eval $(call getcfgvar,CXX))
$(eval $(call getcfgvar,LD))
$(eval $(call getcfgvar,NM))
$(eval $(call getcfgvar,OBJCOPY))
$(eval $(call getcfgvar,CXXFLAGS))
$(eval $(call getcfgvar,LDFLAGS))
endif


ifndef ARCH
$(error must have ARCH environment variable set or .config must exist)
endif

export ARCH

# compiler stuff
ifndef M
CXX      = $(CROSS_COMPILE)g++
LD       = $(CROSS_COMPILE)ld
NM       = $(CROSS_COMPILE)nm
OBJCOPY  = $(CROSS_COMPILE)objcopy
CXXFLAGS = 
LDFLAGS  = 
endif

include scripts/Makefile.build

ifdef BUILD_ARCH
BUILD_SUBDIRS += arch/$(BUILD_ARCH)
include arch/$(BUILD_ARCH)/Makefile.arch
endif

export INT_CXX INT_LD INT_NM INT_OBJCOPY INT_CXXFLAGS INT_LDFLAGS
export CXX LD NM OBJCOPY CXXFLAGS LDFLAGS

# module stuff
ifdef M
include $(M)/Makefile
endif

.PHONY: all
all: syncconfig
ifndef M
	@$(MAKE) kernel
endif

syncconfig:
	@$(MAKE) --quiet -C scripts/kconfig/ config
	@scripts/kconfig/config Kconfig --silent --syncconfig

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
clean: modclean
	@rm -f kernel.o kernel.bin symtab.s symtab.os kernel_partial.o
	@find ./arch ./drivers ./fs ./include ./kernel ./net ./mm ./cppstd ./tests \( -name '*.oasm' -o -name '*.onasm' -o -name '*.ocpp' -o -name '*.ors' -o -name '*.rlib' -o -name '*.o' -o -name '*.os' \) -type f -delete

proper: clean
	@$(MAKE) --quiet -C scripts/kconfig clean
	@rm -f .config .config_old

# recursive stuff - most of this is based on https://stackoverflow.com/q/56001646/20742078
OBJS         :=
MODS         :=
EXTRATARGETS :=
objtree      := .
srctree      := .
define build_subdir
objtree := $$(objtree)/$(1)
srctree := $$(srctree)/$(1)

obj-m :=
obj-y :=
obj-d :=
extratargets-y :=

include $$(srctree)/Makefile

OBJS += $$(patsubst %,$$(objtree)/%,$$(obj-y))
MODS += $$(patsubst %,$$(objtree)/%,$$(obj-m))
EXTRATARGETS += $$(patsubst %,$$(objtree)/%,$$(extratargets-y))

$$(foreach subdir,$$(obj-d),$$(eval $$(call build_subdir,$$(subdir))))

srctree := $$(patsubst %/$(1),%,$$(srctree))
objtree := $$(patsubst %/$(1),%,$$(objtree))
endef

ifndef M
$(foreach dir, $(BUILD_SUBDIRS), $(eval $(call build_subdir,$(dir))))
endif

# final link
.PHONY: kernel
kernel: $(EXTRATARGETS) $(OBJS)
	@$(INT_LD) $(INT_LDFLAGS) -r $(OBJS) -o kernel_partial.o
	@./arch/$(BUILD_ARCH)/archlink.sh

# kernel module stuff
ifdef M
MODS = $(addprefix $(M)/, $(obj-m))
endif

MOD_OBJS := 

define build_mod
MOD_OBJS += $(addprefix $(dir $(1))/,$($(notdir $(basename $(1)))-y))

$(1): $(addprefix $(dir $(1))/,$($(notdir $(basename $(1)))-y))
	@echo "  LD $(notdir $(1))"
	@$(INT_LD) $(INT_LDFLAGS) -r $(addprefix $(dir $(1))/,$($(notdir $(basename $(1)))-y)) -o $(1)
endef

$(foreach dir, $(MODS), $(eval $(call build_mod,$(dir))))

modules: $(MODS)
	@echo "  M: $(notdir $(MODS))"

modclean:
	@rm -f $(MODS) $(MOD_OBJS)
