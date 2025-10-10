obj-d := stdlibs libkernel demo

$(objtree)/rust-target-gen.json:
	@python3 rust/gentarget.py $@

extratargets-$(CONFIG_RUST_SUPPORT) := rust-target-gen.json
