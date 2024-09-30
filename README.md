# vix

![CI](https://github.com/theverygaming/vix/actions/workflows/workflow.yml/badge.svg)

a broken kernel with a ton of bad design choices and copypasted code. _It's actually not that bad anymore tho_

#### Build options

- [Building locally](#building-locally)

#### Goals

- Run X server
- Full Linux binary compatibility
- Binary compatibility with other kernels

References: [PenutOS](https://github.com/AlexandreRouma/PenutOS/), [nanobyte_os](https://github.com/chibicitiberiu/nanobyte_os), the [OSDev wiki](https://wiki.osdev.org), [raspi3-tutorial](https://github.com/bztsrc/raspi3-tutorial), [Building an Operating System for the Raspberry Pi](https://jsandler18.github.io/), [raspberry-pi-os](https://s-matyukevich.github.io/raspberry-pi-os/) and of course [Linux](https://github.com/torvalds/linux)

## Star History

<a href="https://star-history.com/#theverygaming/vix&Date">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=theverygaming/vix&type=Date&theme=dark" />
    <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=theverygaming/vix&type=Date" />
    <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=theverygaming/vix&type=Date" />
  </picture>
</a>

## This is now built using [nix](https://nixos.org/download/), there is a flake.nix but no instructions on how to use it written yet. Take a look at the CI!

## Building locally
### NOTE: THIS IS BE OUTDATED

first make sure you have all submodules. Simply clone like this
```
git clone --recurse-submodules https://github.com/theverygaming/vix.git
```

you already cloned the repo? then run ``git submodule update --init --recursive`` instead

now to build for x86 you need a few extra dependencies(aside from gcc and make), on ubuntu these are installed by running
```
sudo apt install nasm gcc-multilib g++-multilib xorriso grub-common grub-pc mtools python3 cmake pkg-config flex bison libncurses5-dev xxd
```

Now refer to the [general build instructions](#general-build-instructions)

## General build instructions
### NOTE: THIS IS BE OUTDATED

first you have to configure the kernel, you could either simply use the default config (``make alldefconfig``) or configure it yourself (``make menuconfig``)

then you can simply build with ``make`` and then ``make bootimg-x86-32`` (if you have a x86_32 system, otherwise use the ``CROSS_COMPILE`` environment variable or on x86_64 you can use ``CXXFLAGS="-m32 -march=i386" LDFLAGS="-m elf_i386"``)

a bootable iso named ``vix.iso`` will be created

for other architectures look at the [GitHub actions workflow](.github/workflows/workflow.yml)


## clangd how?

```
cd kernel
make clangd_compile_flags_txt
```
