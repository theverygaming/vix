# shitOS
a broken operating system with a ton of bad design choices and copypasted code. _It's actually not that bad anymore tho_

#### Goals
- Full Linux binary compatibility
- Load Linux kernel modules/drivers

References: [PenutOS](https://github.com/AlexandreRouma/PenutOS/), [nanobyte_os](https://github.com/chibicitiberiu/nanobyte_os), the [OSDev wiki](https://wiki.osdev.org), [raspi3-tutorial](https://github.com/bztsrc/raspi3-tutorial), [Building an Operating System for the Raspberry Pi](https://jsandler18.github.io/), [raspberry-pi-os](https://s-matyukevich.github.io/raspberry-pi-os/) and of course [Linux](https://github.com/torvalds/linux)
# how do i build this?

first make sure you have the shitshell submodule. Simply clone like this
```
git clone --recurse-submodules https://github.com/theverygaming/shitOS.git
```

you already cloned the repo? then run ``git submodule update --init --recursive`` instead

now to build for x86 you need a few extra dependencies(aside from gcc and make), on ubuntu these are installed by running
```
sudo apt install nasm gcc-multilib g++-multilib xorriso grub-common grub-pc mtools python3 cmake pkg-config flex bison libncurses5-dev
```

first you have to configure the kernel, you could either simply use the default config (``make alldefconfig``) or configure it yourself (``make menuconfig``)

then you can simply build with ``make`` and ``make img_x86_32``

a bootable iso named ``shitOS.iso`` will be created

for other architectures look at the [GitHub actions workflow](.github/workflows/workflow.yml)
