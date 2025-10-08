lib-$(CONFIG_RUST_SUPPORT) += libkernel.rlib

$(objtree)/libkernel.rlib: $(objtree)/kernel.rs $(objtree)/bindings.rs rust/stdlibs/liballoc.rlib
	$(call build_rust,$<,$@,--crate-name kernel -L $(objtree)/rust/stdlibs,RS_GEN_DEPINFO)

$(objtree)/bindings.rs: $(objtree)/bindings.h
	@bindgen $< --use-core --raw-line $$'#![allow(non_camel_case_types)]\n#![allow(non_snake_case)]' -o $@ -- -x c++ $(INT_CXXFLAGS) --target=$(RUST_TARGET)
	@# FIXME: Problem:
	@# 
	@# for gcc the target i686-elf is used, for clang i686-unknown-linux-gnu (rustup doesn't really offer anything better)
	@# gcc resolves __SIZE_TYPE__ to unsigned long, clang to unsigned int
	@# 
	@# now the kmalloc symbol gets mangled differently between the two and we have a skill issue!
	@# 
	@# Might be an actual issue in clang, but i'm guessing it's because of the target mismatch.. so for now simply replacing it will do the job:tm:
	@# I'll probably get pissed after replacing stuff for a while and fix it
	@sed -i 's/_ZN2mm7kmallocEj/_ZN2mm7kmallocEm/g' $@
	@sed -i 's/_ZN2mm15kmalloc_alignedEjj/_ZN2mm15kmalloc_alignedEmm/g' $@
