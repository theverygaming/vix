FROM mcr.microsoft.com/devcontainers/base:bookworm

RUN apt-get update

# basics
RUN apt-get install --no-install-suggests -y wget bash-completion

# deps
RUN apt-get install --no-install-suggests -y python3 cmake pkg-config flex bison libncurses5-dev xxd

# x86_32
RUN apt-get install --no-install-suggests -y crossbuild-essential-i386 nasm xorriso grub-common grub-pc mtools
RUN apt-get install --no-install-suggests -y qemu-system-x86

# esp32
RUN wget https://github.com/espressif/crosstool-NG/releases/download/esp-13.2.0_20230928/xtensa-esp-elf-13.2.0_20230928-x86_64-linux-gnu.tar.xz
RUN tar -xvf xtensa-esp-elf-13.2.0_20230928-x86_64-linux-gnu.tar.xz
RUN rm xtensa-esp-elf-13.2.0_20230928-x86_64-linux-gnu.tar.xz
ENV PATH="$PATH:/xtensa-esp-elf/bin"
