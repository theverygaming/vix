CC = g++
CFLAGS = -nostdlib -Wall -m32 -march=i386 -fno-pie -fno-stack-protector -ffreestanding

all: stage1 stage2r comp

stage1: stage1.asm
	nasm $< -f bin -o $@

stage2r:
	$(MAKE) -C stage2

comp: stage1 stage2
	#cat stage1 stage2/stage2 > boot
	cat stage1 > boot

%.ocpp: %.cpp $(HEADERS_CPP)
	$(CC) $(CFLAGS) -c $< -o $@

%.oasm: %.asm
	nasm -f elf -o $@ $^ 

clean:
	rm -f boot stage1 *.oasm *.ocpp *.o
	$(MAKE) -C stage2 clean