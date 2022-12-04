# shitOS
a broken operating system with a ton of bad design choices and copypasted code

#### Goals
- run Linux applications with little modifications
- be able to main this(will take 10+ years probably)

References: [PenutOS](https://github.com/AlexandreRouma/PenutOS/), [nanobyte_os](https://github.com/chibicitiberiu/nanobyte_os), the [OSDev wiki](https://wiki.osdev.org), [raspi3-tutorial](https://github.com/bztsrc/raspi3-tutorial) and of course [Linux](https://github.com/torvalds/linux)

# how do i build this?

first make sure you have the shitshell submodule. Simply clone like this
```
git clone --recurse-submodules https://github.com/theverygaming/shitOS.git
```

you already cloned the repo? then run ``git submodule update --init`` instead

now to build for x86 you need a few extra dependencies(aside from gcc and make), on ubuntu these are installed by running
```
sudo apt install nasm gcc-multilib g++-multilib xorriso grub-common grub-pc mtools python3
```

then you can simply build with
```
make MAKE_ARCH=x86
```

a bootable iso named ``shitOS.iso`` will be created
