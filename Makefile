export MAKE_ARCH

all: img-$(MAKE_ARCH)

img-x86:
	@$(MAKE) --no-print-directory -C kernel
	@$(MAKE) --no-print-directory -C startup/$(MAKE_ARCH)
	@$(MAKE) --no-print-directory -C shitshell
	@$(MAKE) --no-print-directory -C modules
	@cat startup/$(MAKE_ARCH)/startup kernel/kernel.bin /dev/zero | dd status=none iflag=fullblock of=kernel.bin bs=65536 count=7 # make the ramfs land right at KERNEL_CODE_SIZE
	@g++ roramfs_create.cpp -o roramfs_create
	@./roramfs_create roramfs.fs "insert fs label here" shitshell/shitshell modules/module.o
	@cat kernel.bin roramfs.fs /dev/zero | dd status=none iflag=fullblock of=kernel_shitshell.bin bs=65536 count=74
	@grub/createimg.sh

img-aarch64:
	@$(MAKE) --no-print-directory -C startup/$(MAKE_ARCH)

img-lx106_esp8266:
	@$(MAKE) --no-print-directory -C kernel

config:
	@$(MAKE) --no-print-directory -C kernel config

clean:
	@rm -f shitOS.img shitOS_app.img *.o
	@$(MAKE) --no-print-directory -C kernel clean
	@$(MAKE) --no-print-directory -C startup/$(MAKE_ARCH) clean # lx106 skill issue
	@$(MAKE) --no-print-directory -C shitshell clean
	@$(MAKE) --no-print-directory -C modules clean

proper: clean
	@$(MAKE) --no-print-directory -C kernel proper
	@$(MAKE) --no-print-directory -C startup/$(MAKE_ARCH) proper
