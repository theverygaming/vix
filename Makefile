MAKE_ARCH ?= x86
ARCH := $(MAKE_ARCH)
export MAKE_ARCH
export ARCH

all: kernel-$(MAKE_ARCH)

alldefconfig:
	@$(MAKE) --no-print-directory -C kernel alldefconfig

defconfig:
	@$(MAKE) --no-print-directory -C kernel defconfig

savedefconfig:
	@$(MAKE) --no-print-directory -C kernel savedefconfig

syncconfig:
	@$(MAKE) --no-print-directory -C kernel syncconfig

tests:
	@cd kernel && echo "CONFIG_ENABLE_TESTS=y" >> .config # hacky but "works"
	$(MAKE) syncconfig

menuconfig:
	@$(MAKE) --no-print-directory -C kernel menuconfig

bootimg-x86-32:
	@$(MAKE) --no-print-directory -C shitshell
	@$(MAKE) --no-print-directory -C kernel/ M=$(PWD)/modules modules
	@#g++ -m32 -march=i386 -static -static-libgcc -static-libstdc++ -mno-red-zone -fno-pie -fno-stack-protector tools/glibctest.cpp -o libctest
	@#gcc -m32 -march=i386 -static -static-libgcc -mno-red-zone -fno-pie -fno-stack-protector tools/glibctest.c -o libctest -pthread
	@#musl-gcc -m32 -march=i386 -static -static-libgcc -mno-red-zone -fno-pie -fno-stack-protector tools/glibctest.c -o libctest
	@mkdir -p sysroot/usr/share/consolefonts/ sysroot/usr/lib/modules sysroot/bin/
	@cp shitshell/shitshell sysroot/bin/sh
	@cp modules/module.o modules/module2.o sysroot/usr/lib/modules
	@cp kernel/drivers/intree-mod.o sysroot/usr/lib/modules || true
	@cp fonts/Unifont-APL8x16-15.0.01.psf sysroot/usr/share/consolefonts
	@find sysroot/ -printf "%P\n" | tar --format=ustar -cf roramfs.fs --no-recursion -C sysroot/ -T -
	@boot/createimg-x86_32.sh

bootimg-x86-64:
	@mkdir -p sysroot/usr/share/consolefonts/
	@cp fonts/Unifont-APL8x16-15.0.01.psf sysroot/usr/share/consolefonts
	@find sysroot/ -printf "%P\n" | tar --format=ustar -cf roramfs.fs --no-recursion -C sysroot/ -T -
	@boot/createimg-x86_64.sh

bootimg-m68k:
	@touch fs.img
	@truncate --size 1M fs.img
	@mkfs.fat -F12 fs.img -v
	@mmd -i fs.img ::boot
	@mcopy -i fs.img kernel/kernel.bin ::/boot/kernel
	@mcopy -i fs.img ../macboot/example/boot.cfg ::/
	@mdir -i fs.img ::/
	@mdir -i fs.img ::/boot
	@cat ../macboot/startup fs.img > disk.img

bootimg-aarch64:
	@mkdir -p sysroot/usr/share/consolefonts/
	@cp fonts/Unifont-APL8x16-15.0.01.psf sysroot/usr/share/consolefonts
	@find sysroot/ -printf "%P\n" | tar --format=ustar -cf roramfs.fs --no-recursion -C sysroot/ -T -
	@boot/createimg-aarch64.sh

kernel-x86:
	@$(MAKE) --no-print-directory -C kernel

kernel-xtensa:
	@$(MAKE) --no-print-directory -C kernel

kernel-aarch64:
	@$(MAKE) --no-print-directory -C kernel

kernel-m68k:
	@$(MAKE) --no-print-directory -C kernel

clean-xtensa:
clean-m68k:
clean-aarch64:
clean-x86:

clean: clean-$(MAKE_ARCH)
	@rm -f vix.img vix.iso *.o
	@$(MAKE) --no-print-directory -C kernel clean
	@$(MAKE) --no-print-directory -C shitshell clean
	@$(MAKE) --no-print-directory -C kernel/ M=$(PWD)/modules clean

distclean: clean
	@$(MAKE) --no-print-directory -C kernel distclean
	@rm -rf sysroot/
