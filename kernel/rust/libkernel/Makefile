lib-$(CONFIG_RUST_SUPPORT) += rust.ocpp libkernel.rlib $(wildcard $(shell rustc --print sysroot)/lib/rustlib/$(RUST_TARGET)/lib/libcore*.rlib) $(wildcard $(shell rustc --print sysroot)/lib/rustlib/$(RUST_TARGET)/lib/liballoc*.rlib)

$(objtree)/libkernel.rlib: $(objtree)/kernel.rs $(objtree)/bindings.rs
	$(call build_rust,$<,$@,)

$(objtree)/bindings.rs: $(objtree)/bindings.h
	@bindgen $< --use-core --raw-line $$'#![allow(non_camel_case_types)]\n#![allow(non_snake_case)]' -o $@ -- -x c++ $(INT_CXXFLAGS) --target=$(RUST_TARGET)
	@# Problem:
	@# 
	@# for gcc the target i686-elf is used, for clang i686-unknown-linux-gnu (rustup doesn't really offer anything better)
	@# gcc resolves __SIZE_TYPE__ to unsigned long, clang to unsigned int
	@# 
	@# now the kmalloc symbol gets mangled differently between the two and we have a skill issue!
	@# 
	@# Might be an actual issue in clang, but i'm guessing it's because of the target mismatch.. so for now simply replacing it will do the job:tm:
	@sed -i 's/_ZN2mm7kmallocEj/_ZN2mm7kmallocEm/g' $@
