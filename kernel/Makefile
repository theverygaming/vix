CC = g++
CFLAGS = -nostdlib -Wall -m32 -march=i386 -fno-pie -fno-stack-protector -ffreestanding -fno-exceptions -O3

HEADERS_CPP = $(wildcard *.h) \
			$(wildcard */*.h) \
			$(wildcard */*/*.h) \
			$(wildcard */*/*/*.h)

SOURCES_CPP = $(wildcard *.cpp) \
			$(wildcard */*.cpp) \
			$(wildcard */*/*.cpp) \
			$(wildcard */*/*/*.cpp)

OBJECTS_CPP = $(patsubst %.cpp, %.ocpp, $(SOURCES_CPP))

HEADERS_ASM = $(wildcard *.inc) \
			  $(wildcard */*.inc) \
			  $(wildcard */*/*.inc) \
			  $(wildcard */*/*/*.inc)

SOURCES_ASM = $(wildcard *.asm) \
			  $(wildcard */*.asm) \
			  $(wildcard */*/*.asm) \
			  $(wildcard */*/*/*.asm)

OBJECTS_ASM = $(patsubst %.asm, %.oasm, $(SOURCES_ASM))

all: kernel

kernel: kernel.ocpp $(OBJECTS_ASM) $(OBJECTS_CPP)
	ld -m elf_i386 --oformat binary -T linker.ld -nostdlib $^ -o $@

%.ocpp: %.cpp $(HEADERS_CPP)
	$(CC) $(CFLAGS) -c $< -o $@ 

%.oasm: %.asm
	nasm -f elf -o $@ $^ 

clean:
	rm -f kernel
	find . \( -name '*.oasm' -o -name '*.ocpp' \) -type f -delete