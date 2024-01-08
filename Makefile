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

img_x86_32:
	@$(MAKE) --no-print-directory -C prekernel/$(MAKE_ARCH)
	@$(MAKE) --no-print-directory -C shitshell
	@$(MAKE) --no-print-directory -C kernel/ M=$(PWD)/modules modules
	@cat prekernel/$(MAKE_ARCH)/startup kernel/kernel.bin /dev/zero | dd status=none iflag=fullblock of=kernel.bin bs=65536 count=16 # make the ramfs land right at KERNEL_CODE_SIZE
	@#g++ -m32 -march=i386 -static -static-libgcc -static-libstdc++ -mno-red-zone -fno-pie -fno-stack-protector tools/glibctest.cpp -o libctest
	@#gcc -m32 -march=i386 -static -static-libgcc -mno-red-zone -fno-pie -fno-stack-protector tools/glibctest.c -o libctest -pthread
	@#musl-gcc -m32 -march=i386 -static -static-libgcc -mno-red-zone -fno-pie -fno-stack-protector tools/glibctest.c -o libctest
	@g++ tools/roramfs_create.cpp -o roramfs_create
	@#./roramfs_create roramfs.fs "insert fs label here" shitshell/shitshell modules/module.o fonts/Unifont-APL8x16-15.0.01.psf
	@mkdir -p sysroot/usr/share/consolefonts/ sysroot/usr/lib/modules sysroot/bin/
	@cp shitshell/shitshell sysroot/bin/sh
	@cp modules/module.o sysroot/usr/lib/modules
	@cp fonts/Unifont-APL8x16-15.0.01.psf sysroot/usr/share/consolefonts
	@find sysroot/ -printf "%P\n" | tar --format=ustar -cf roramfs.fs --no-recursion -C sysroot/ -T -
	@cat kernel.bin roramfs.fs /dev/zero | dd status=none iflag=fullblock of=kernel_shitshell.bin bs=65536 count=83
	@boot/createimg-x86_32.sh

img_x86_64:
	@g++ tools/roramfs_create.cpp -o roramfs_create
	@./roramfs_create roramfs.fs "insert fs label here" fonts/Unifont-APL8x16-15.0.01.psf
	@boot/createimg-x86_64.sh

img-x86:
	@$(MAKE) --no-print-directory -C kernel

img-xtensa:
	@$(MAKE) --no-print-directory -C kernel

img-aarch64:
	@$(MAKE) --no-print-directory -C kernel
	@g++ tools/roramfs_create.cpp -o roramfs_create
	@./roramfs_create roramfs.fs "insert fs label here" fonts/Unifont-APL8x16-15.0.01.psf
	@boot/createimg-aarch64.sh

img-m68k:
	@$(MAKE) --no-print-directory -C kernel

clean-xtensa:
clean-m68k:
clean-aarch64:
clean-x86:
	@$(MAKE) --no-print-directory -C prekernel/$(MAKE_ARCH) clean
	@$(MAKE) --no-print-directory -C prekernel/$(MAKE_ARCH) proper

clean: clean-$(MAKE_ARCH)
	@rm -f vix.img vix.iso *.o
	@$(MAKE) --no-print-directory -C kernel clean
	@$(MAKE) --no-print-directory -C shitshell clean
	@$(MAKE) --no-print-directory -C kernel/ M=$(PWD)/modules clean

proper: clean
	@$(MAKE) --no-print-directory -C kernel proper
