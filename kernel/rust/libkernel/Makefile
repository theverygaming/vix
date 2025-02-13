lib-$(CONFIG_RUST_SUPPORT) += rust.ocpp libkernel.rlib $(wildcard $(shell rustc --print sysroot)/lib/rustlib/$(RUST_TARGET)/lib/libcore*.rlib) $(wildcard $(shell rustc --print sysroot)/lib/rustlib/$(RUST_TARGET)/lib/liballoc*.rlib)

$(objtree)/libkernel.rlib: $(objtree)/kernel.rs $(objtree)/bindings.rs
	$(call build_rust,$<,$@,)

$(objtree)/bindings.rs: $(objtree)/bindings.h
	@bindgen $< --use-core --raw-line $$'#![allow(non_camel_case_types)]\n#![allow(non_snake_case)]' -o $@ -- -x c++ $(INT_CXXFLAGS)
