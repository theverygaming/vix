# erase defaults
CXX=
LD=

include Makefile.version

BUILD_SUBDIRS := kernel drivers fs net tests mm stdlibs abi rust

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
$(eval $(call getcfgvar,CFLAGS))
$(eval $(call getcfgvar,CXXFLAGS))
$(eval $(call getcfgvar,LDFLAGS))
endif


ifndef ARCH
$(error must have ARCH environment variable set or .config must exist)
endif

export ARCH

# compiler stuff
ifndef M
CC      = $(CROSS_COMPILE)gcc
CXX      = $(CROSS_COMPILE)g++
LD       = $(CROSS_COMPILE)ld
NM       = $(CROSS_COMPILE)nm
OBJCOPY  = $(CROSS_COMPILE)objcopy
CFLAGS = 
CXXFLAGS = 
LDFLAGS  = 
endif

include scripts/Makefile.build

ifdef BUILD_ARCH
BUILD_SUBDIRS += arch/$(BUILD_ARCH)
include arch/$(BUILD_ARCH)/Makefile.arch
endif

export INT_CC INT_CXX INT_LD INT_NM INT_OBJCOPY INT_CFLAGS INT_CXXFLAGS INT_LDFLAGS
export CXX LD NM OBJCOPY CFLAGS CXXFLAGS LDFLAGS
export BUILD_ARCH

# module stuff
ifdef M
include $(M)/Makefile
endif

.PHONY: all
all: syncconfig
ifndef M
	@$(MAKE) kernel
endif

clangd_compile_flags_txt:
	@echo -working-directory=$(CURDIR) > compile_flags.txt
	@echo --language=c++ >> compile_flags.txt
	@echo "${INT_CXXFLAGS}" | sed -e 's/ /\n/g' >> compile_flags.txt

scripts/kconfig/Makefile:
	@curl https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.7.5.tar.xz -o linux-6.7.5.tar.xz
	@tar -xvf linux-6.7.5.tar.xz linux-6.7.5/scripts/kconfig/ -C scripts/kconfig --strip-components=1
	@rm -f linux-6.7.5.tar.xz
	@cp scripts/kconfig_Makefile scripts/kconfig/Makefile

syncconfig: scripts/kconfig/Makefile
	@$(MAKE) --quiet -C scripts/kconfig/ config
	@scripts/kconfig/config Kconfig --silent --syncconfig

help: scripts/kconfig/Makefile
	@echo 'Cleaning targets:'
	@echo '  clean           - remove everything not required for building modules'
	@echo '  distclean          - clean + remove everything generated'
	@echo 'Configuration targets:'
	@$(MAKE) --quiet -C scripts/kconfig/ help
	@echo ''
	@echo 'Execute "make" or "make all" to build all targets marked with [*]'

# kconfig targets
config: scripts/kconfig/Makefile
	@$(MAKE) --quiet -C scripts/kconfig/ config
	@scripts/kconfig/config Kconfig --oldaskconfig

nconfig: scripts/kconfig/Makefile
	@$(MAKE) --quiet -C scripts/kconfig/ nconfig
	@scripts/kconfig/nconfig Kconfig

menuconfig: scripts/kconfig/Makefile
	@$(MAKE) --quiet -C scripts/kconfig/ menuconfig
	@scripts/kconfig/menuconfig Kconfig

xconfig: scripts/kconfig/Makefile
	@$(MAKE) --quiet -C scripts/kconfig/ xconfig
	@scripts/kconfig/xconfig Kconfig

gconfig: scripts/kconfig/Makefile
	@$(MAKE) --quiet -C scripts/kconfig/ gconfig
	@scripts/kconfig/gconfig Kconfig

alldefconfig: scripts/kconfig/Makefile
	@$(MAKE) --quiet -C scripts/kconfig/ config
	@scripts/kconfig/config Kconfig --silent --alldefconfig

# clean targets
clean: modclean
	@rm -f kernel.o kernel.bin symtab.S symtab.oS kernel_partial.o
	@find ./arch ./drivers ./fs ./include ./kernel ./net ./mm ./stdlibs ./tests ./abi ./rust \( -name '*.oasm' -o -name '*.onasm' -o -name '*.oc' -o -name '*.ocpp' -o -name '*.o' -o -name '*.os' -o -name '*.oS' -o -name '*.rlib' \) -type f -delete

distclean: clean
	@rm -f .config .config_old
	@rm -rf scripts/kconfig

# recursive stuff - most of this is based on https://stackoverflow.com/q/56001646/20742078
OBJS         :=
LIBS         :=
MODS         :=
EXTRATARGETS :=
objtree      := .
srctree      := .
define build_subdir
objtree := $$(objtree)/$(1)
srctree := $$(srctree)/$(1)

obj-m :=
obj-y :=
lib-y := 
obj-d :=
extratargets-y :=

include $$(srctree)/Makefile

# obj-y, lib-y etc. add $(objtree) to non-absolute paths
OBJS += $$(patsubst %,$$(objtree)/%,$$(filter-out /%,$$(obj-y))) $$(filter /%,$$(obj-y))
LIBS += $$(patsubst %,$$(objtree)/%,$$(filter-out /%,$$(lib-y))) $$(filter /%,$$(lib-y))
MODS += $$(patsubst %,$$(objtree)/%,$$(filter-out /%,$$(obj-m))) $$(filter /%,$$(obj-m))
EXTRATARGETS += $$(patsubst %,$$(objtree)/%,$$(filter-out /%,$$(extratargets-y))) $$(filter /%,$$(extratargets-y))

$$(foreach subdir,$$(obj-d),$$(eval $$(call build_subdir,$$(subdir))))

srctree := $$(patsubst %/$(1),%,$$(srctree))
objtree := $$(patsubst %/$(1),%,$$(objtree))
endef

ifndef M
$(foreach dir, $(BUILD_SUBDIRS), $(eval $(call build_subdir,$(dir))))
endif

.PHONY: pre_build
pre_build: $(EXTRATARGETS)

.PHONY: build_libs
build_libs: $(LIBS)

.PHONY: build_objs
build_objs: $(OBJS)

# final link
# use .WAIT when GNU Make 4.4 is finally common
.PHONY: kernel
kernel: | pre_build build_libs build_objs
	@$(INT_LD) $(INT_LDFLAGS) -r $(OBJS) $(LIBS) -o kernel_partial.o
	@./arch/$(BUILD_ARCH)/archlink.sh

# kernel module stuff
ifdef M
MODS = $(addprefix $(M)/, $(obj-m))
endif

MOD_OBJS := 

define build_mod
MOD_OBJS += $(addprefix $(dir $(1)),$($(notdir $(basename $(1)))-y))

$(1): $(addprefix $(dir $(1)),$($(notdir $(basename $(1)))-y))
	@echo "  LD $(notdir $(1))"
	@$(INT_LD) $(INT_LDFLAGS) -r $(addprefix $(dir $(1)),$($(notdir $(basename $(1)))-y)) -o $(1)
endef

$(foreach dir, $(MODS), $(eval $(call build_mod,$(dir))))

modules: $(MODS)
	@echo "  M: $(notdir $(MODS))"

modclean:
	@rm -f $(MODS) $(MOD_OBJS)
