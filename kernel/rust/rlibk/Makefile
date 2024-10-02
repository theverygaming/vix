lib-$(CONFIG_RUST_SUPPORT) += librlibk.a

# FIXME: figure out the rust target stuff
RUST_TARGET := i686-unknown-linux-gnu

$(objtree)/librlibk.a: $(objtree)/src/**
	@#FIXME: get rid of the copy, stupid
	@cd $(dir $@) && cargo build --release --target $(RUST_TARGET)
	@cp $(dir $@)target/$(RUST_TARGET)/release/librlibk.a $@
