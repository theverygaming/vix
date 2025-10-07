lib-$(CONFIG_RUST_SUPPORT) += libcore.rlib libcompiler_builtins.rlib liballoc.rlib

RUST_CORE_LIB_SRC := $(shell rustc --print sysroot)/lib/rustlib/src/rust/library

$(objtree)/libcore.rlib: $(RUST_CORE_LIB_SRC)/core/src/lib.rs
	$(call build_rust,$<,$@,--crate-name core --cfg no_fp_fmt_parse)

$(objtree)/libcompiler_builtins.rlib: $(objtree)/compiler_builtins.rs $(objtree)/libcore.rlib
	$(call build_rust,$<,$@,-L $(objtree)/rust/stdlibs --crate-name compiler_builtins --extern core=$(objtree)/rust/stdlibs/libcore.rlib)

$(objtree)/liballoc.rlib: $(RUST_CORE_LIB_SRC)/alloc/src/lib.rs $(objtree)/libcore.rlib $(objtree)/libcompiler_builtins.rlib
	$(call build_rust,$<,$@,-L $(objtree)/rust/stdlibs --crate-name alloc --extern core=$(objtree)/rust/stdlibs/libcore.rlib --extern compiler_builtins=$(objtree)/rust/stdlibs/libcompiler_builtins.rlib)
