CC = gcc
CFLAGS = -nostdlib -Wall -m32 -march=i386 -fno-pie -fno-stack-protector -ffreestanding

all: shitshell

shitshell: start.oasm start.oc main.oc syscall.oasm syscall.oc stdlib.oc
	ld -m elf_i386 -nostdlib $^ -o $@

%.oc: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.oasm: %.asm
	nasm -f elf32 -o $@ $^

clean:
	rm -f shitshell *.oasm *.oc *.o