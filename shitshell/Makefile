CC = g++
CFLAGS = -nostdlib -Wall -m32 -march=i386 -fno-pie -fno-stack-protector -ffreestanding

all: shitshell

shitshell: shitshell.oasm
	ld -m elf_i386 -nostdlib $^ -o $@

%.ocpp: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

%.oasm: %.asm
	nasm -f elf32 -o $@ $^

clean:
	rm -f shitshell *.oasm *.ocpp *.o