CC = g++
CFLAGS = -nostdlib -Wall -m32 -march=i386 -fno-pie -fno-stack-protector -ffreestanding

all: stage2

stage2: stage2.ocpp
	ld -m elf_i386 --oformat binary -Ttext 1000 $^ -o $@

%.ocpp: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

%.oasm: %.asm
	nasm -f elf -o $@ $^ 

clean:
	rm -f stage2 *.oasm *.ocpp *.o