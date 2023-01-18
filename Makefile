MAKE_ARCH ?= x86
ARCH := $(MAKE_ARCH)
export MAKE_ARCH
export ARCH

all: img-$(MAKE_ARCH)

alldefconfig:
	@$(MAKE) --no-print-directory -C kernel alldefconfig

tests:
	@cd kernel && echo "CONFIG_ENABLE_TESTS=y" >> .config # hacky but "works"

menuconfig:
	@$(MAKE) --no-print-directory -C kernel menuconfig

img-x86:
	@$(MAKE) --no-print-directory -C kernel
	@$(MAKE) --no-print-directory -C startup/$(MAKE_ARCH)
	@$(MAKE) --no-print-directory -C shitshell
	@$(MAKE) --no-print-directory -C modules
	@cat startup/$(MAKE_ARCH)/startup kernel/kernel.bin /dev/zero | dd status=none iflag=fullblock of=kernel.bin bs=65536 count=16 # make the ramfs land right at KERNEL_CODE_SIZE
	@g++ tools/roramfs_create.cpp -o roramfs_create
	@#g++ -m32 -march=i386 -static -static-libgcc -static-libstdc++ -mno-red-zone -fno-pie -fno-stack-protector tools/glibctest.cpp -o libctest
	@gcc -m32 -march=i386 -static -static-libgcc -mno-red-zone -fno-pie -fno-stack-protector tools/glibctest.c -o libctest -pthread
	@#musl-gcc -m32 -march=i386 -static -static-libgcc -mno-red-zone -fno-pie -fno-stack-protector tools/glibctest.c -o libctest
	@./roramfs_create roramfs.fs "insert fs label here" shitshell/shitshell modules/module.o libctest fonts/Unifont-APL8x16-15.0.01.psf
	@cat kernel.bin roramfs.fs /dev/zero | dd status=none iflag=fullblock of=kernel_shitshell.bin bs=65536 count=83
	@boot/createimg.sh

img-aarch64:
	@$(MAKE) --no-print-directory -C kernel

img-lx106_esp8266:
	@$(MAKE) --no-print-directory -C kernel

clean-lx106_esp8266:
clean-aarch64:
clean-x86:
	@$(MAKE) --no-print-directory -C startup/$(MAKE_ARCH) clean
	@$(MAKE) --no-print-directory -C startup/$(MAKE_ARCH) proper

clean: clean-$(MAKE_ARCH)
	@rm -f shitOS.img shitOS.iso *.o
	@$(MAKE) --no-print-directory -C kernel clean
	@$(MAKE) --no-print-directory -C shitshell clean
	@$(MAKE) --no-print-directory -C modules clean

proper: clean
	@$(MAKE) --no-print-directory -C kernel proper
