MAKE_ARCH := x86
export MAKE_ARCH

all: img

img:
	@$(MAKE) --no-print-directory -C startup/$(MAKE_ARCH)
	@$(MAKE) --no-print-directory -C kernel
	@$(MAKE) --no-print-directory -C shitshell
	@cat startup/$(MAKE_ARCH)/startup kernel/kernel.bin /dev/zero | dd status=none iflag=fullblock of=kernel.bin bs=65536 count=246 # make the shitshell executable land right at KERNEL_FREE_AREA_BEGIN_OFFSET
	@g++ roramfs_create.cpp -o roramfs_create
	@./roramfs_create roramfs.fs "insert fs label here" shitshell/shitshell
	@cat kernel.bin roramfs.fs /dev/zero | dd status=none iflag=fullblock of=kernel_shitshell.bin bs=65536 count=290
	@grub/createimg.sh

config:
	@$(MAKE) --no-print-directory -C kernel config

clean:
	@rm -f shitOS.img shitOS_app.img *.o
	@$(MAKE) --no-print-directory -C kernel clean
	@$(MAKE) --no-print-directory -C startup/$(MAKE_ARCH) clean
	@$(MAKE) --no-print-directory -C shitshell clean
