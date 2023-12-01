FROM mcr.microsoft.com/devcontainers/base:bookworm

RUN apt-get update

# usability
RUN apt-get install -y bash-completion

# x86_32
RUN apt-get install -y crossbuild-essential-i386
RUN apt-get install -y qemu-system-x86
