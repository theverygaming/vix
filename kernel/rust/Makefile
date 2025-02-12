lib-$(CONFIG_RUST_SUPPORT) += libkernel.rlib $(wildcard $(shell rustc --print sysroot)/lib/rustlib/$(RUST_TARGET)/lib/libcore*.rlib)

$(objtree)/libkernel.rlib: $(objtree)/kernel.rs
	$(call build_rust, $<, $@)
