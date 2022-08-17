CC = g++
CFLAGS = -nostdlib -Wall -m32 -march=i386 -fno-pie -fno-stack-protector -ffreestanding

all: 
	@$(MAKE) stage2
	@dd status=none if=/dev/zero of=stage2 bs=1 count=1 seek=10239

stage2: stage2.ocpp gdt.oasm gdt.ocpp paging.ocpp paging.oasm stdio.ocpp hdd.ocpp stdlib.ocpp
	@ld -m elf_i386 --oformat binary -T linker.ld -nostdlib $^ -o $@

%.ocpp: %.cpp
	@echo "-> compiling boot/stage2/$<"
	@$(CC) $(CFLAGS) -c $< -o $@

%.oasm: %.asm
	@echo "-> compiling boot/stage2/$<"
	@nasm -f elf -o $@ $^

clean:
	@rm -f stage2 *.oasm *.ocpp *.o